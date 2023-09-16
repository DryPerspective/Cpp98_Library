#ifndef CPP98_OPTIONAL
#define CPP98_OPTIONAL

#include <new>
#include <exception>
#include <algorithm> //For std::swap, which lived in <algorithm> before C++11

/*
*	An analogue of std::optional which should work as far back as C++98.
*
*   Implementation decisions:
*   - We can't use alignment operators to police memory as they are a C++11 feature
*   - We can't use a union containing T directly, as pre-C++11 unions were not permitted to contain
*     types with nontrivial member functions
*
*   Implementation notes:
*	- It's unavoidable that without memory laundering the lifetimes of this one are technically UB.
*	  As much as it pains me to say it, this is probably the kind of UB which "just works" and can stay in.
*/

//It's better to use std::optional and you should be able to swap this out for it when you get to C++17

namespace dp{

//A token "tag" to represent an empty optional
struct nullopt_t{};
//Can't to the old extern instance trick - the C++Builder linker says no
static const nullopt_t nullopt = {};


//Bad optional access exception
struct bad_optional_access : std::exception {
	bad_optional_access() {}
	virtual ~bad_optional_access() throw() {}
	virtual const char* what() const throw() {
		return "Bad optional access";
	}
};


template<typename T>
class optional{

	typedef T contained_type; //Typedef to avoid two-phase lookup issues

	//Union trick to delay initialization
	union {
		unsigned char m_Storage[sizeof(contained_type)];    //The data representing the object.
		long double m_phony_max_align;          			//Hacky analogue of std::max_align_t
	};
	bool      m_HasValue;

	//Cut down on reinterpret_casts all over the place
	inline T& storedObject(){
		return *reinterpret_cast<T*>(m_Storage);
	}

	inline const T& storedObject() const{
		return *reinterpret_cast<const T*>(m_Storage);
	}



public:

	//Public facing typedef. Less ugly than public-private-public switcheroo to use the other one
	typedef T value_type;

    optional() : m_HasValue(false) {}
	optional(nullopt_t) : m_HasValue(false) {}

	optional(const optional& in) : m_HasValue(in.m_HasValue) {
		//Not initialized so no risk of overwriting a valid object
		if (m_HasValue) new (m_Storage) T(*in);
	}

	template<typename U>
	explicit optional(const optional<U>& in) : m_HasValue(in.has_value()) {
		if(m_HasValue) new (m_Storage) T(*in);
	}

	template<typename U>
	optional(const U& Value) : m_HasValue(true){
		new (m_Storage) T(Value);
	}

    ~optional()
	{
		this->reset();
	}

	optional& operator=(nullopt_t) {
		this->reset();
		return *this;
	}
	
	template<typename U>
    optional& operator=(const U& in) {
		if (m_HasValue) storedObject() = in;
		else{
			new (m_Storage) T(in);
			m_HasValue = true;
		}
		return *this;
	}

	template<typename U>
	optional& operator=(const optional<U>& in){
	//Note that optional::swap's exception specification depends on both swapping and copying.
	//Copy-and-swap should still maintain the strong exception guarantee here. Assuming no abnormal exception behaviour
	//which would also trip up a usual copy-and-swap (e.g. throwing destructor), the latest an exception can be thrown is 
	//in the first operation within swap(), at which point the original state in both cases will not have been modified
	//Since those operations are either a swap (which should be noexcept) or a placement new into uninitialized memory,
	//the meaningful state of the program should not change
		  optional<T> copy(in);
		  this->swap(copy);
		  return *this;
	}



	void reset(){
 		if(m_HasValue){
			this->storedObject().~T();
			m_HasValue = false;
		}
	}

    //Note we can't use a general template here because
    //std::swap doesn't work for multiple types, even convertible ones
	void swap(dp::optional<T>& other){
		//Because of the possibility of one or more object being uninitialized, this can't be a simple swap
		//As such its noexcept specification isn't simple either
		//This is non-throwing if swap(T,T) is nonthrowing and if T is nothrow-copy-constructible.

		/*
		*   Four Important possibilities
		*   1: Both optionals have a value - we need a value-wise swap not a bitwise one
		*   2: We have a value but are swapped with an empty.
		*   3: We don't have a value but the other does - can't dereference an uninitialized pointer so we placement new
		*   4: Neither has a value so we don't need to do anything anyway
		*/
        using std::swap;	//Two-step swap, and the m_HasValue needs std::swap anyway
		if(m_HasValue && other.has_value()){
			swap(this->storedObject(), other.storedObject());
		}
		else if(m_HasValue){
			new (other.m_Storage) T(this->storedObject());
			this->reset();
		}
		else if(other.m_HasValue) {
			new (m_Storage) T( other.storedObject() );
			other.reset();
		}

		swap(m_HasValue, other.m_HasValue);
	}

	const T& operator*() const{
		return storedObject();
    }

	T& operator*(){
		return storedObject();
	}

	const T* operator->() const{
		return &(storedObject());
	}

	T* operator->(){
		return &(storedObject());
	}

    bool has_value() const { return m_HasValue; }
	//operator bool() const { return has_value(); }

	T& value(){
		if (!this->has_value()) throw dp::bad_optional_access();
		return storedObject();
	}

	const T& value() const{
		if (!this->has_value()) throw dp::bad_optional_access();
		return storedObject();
	}

	//Alas no move semantics/forwarding references
	template<typename Other>
	T value_or(const Other& other){
		if(has_value()) return storedObject();
		else return other;
	}


};

//Left undefined to prevent use, for obvious reasons
template<>
class optional<nullopt_t>;

template<typename T>
class optional<T&>;


//And a swap in the same namespace, with the same exception specification as before
//Noexcept for swap(T,T) and T's copy constructor.
template<typename T>
void swap(optional<T>& lhs, optional<T>& rhs){
    lhs.swap(rhs);
}

template<typename T>
optional<T> make_optional() {
	return optional<T>();
}

template<typename T, typename U>
optional<T> make_optional(const U& in) {
	return optional<T>(in);
}

//Comparison operators
//Friend members pattern to allow implicit conversion of solid values with optional ones
template<typename T, typename U>
bool operator==(const optional<T>& lhs, const optional<U>& rhs) {
	if (lhs && rhs) {
		return *lhs == *rhs;
	}
	else return !lhs && !rhs;
}
template<typename T, typename U>
 bool operator!=(const optional<T>& lhs, const optional<U>& rhs) {
	return !(lhs == rhs);
}
template<typename T, typename U>
bool operator<(const optional<T>& lhs, const optional<U>& rhs) {
	if (lhs && rhs) {
		return *lhs < *rhs;
	}
	return !lhs && rhs;
}
template<typename T, typename U>
bool operator<=(const optional<T>& lhs, const optional<U>& rhs) {
	return lhs < rhs || lhs == rhs;
}
template<typename T, typename U>
bool operator>(const optional<T>& lhs, const optional<U>& rhs) {
	return !(lhs <= rhs);
}
template<typename T, typename U>
bool operator>=(const optional<T>& lhs, const optional<U>& rhs) {
	return !(lhs < rhs);
}

template<typename T>
bool operator==(const dp::optional<T>& lhs, dp::nullopt_t){
	return !lhs.has_value();
}
template<typename T>
bool operator==(dp::nullopt_t lhs, const dp::optional<T>& rhs) {
	return rhs == lhs;
}
template<typename T>
bool operator<(const dp::optional<T>&, dp::nullopt_t) {
	return false;
}
template<typename T>
bool operator<(dp::nullopt_t, const dp::optional<T>& rhs) {
	return rhs.has_value();
}
template<typename T>
bool operator<=(const dp::optional<T>& lhs, dp::nullopt_t) {
	return !lhs.has_value();
}
template<typename T>
bool operator<=(dp::nullopt_t, const dp::optional<T>&) {
	return true;
}
template<typename T>
bool operator>(const dp::optional<T>& lhs, dp::nullopt_t) {
	return lhs.has_value();
}
template<typename T>
bool operator>(dp::nullopt_t, const dp::optional<T>&) {
	return false;
}
template<typename T>
bool operator>=(const dp::optional<T>&, dp::nullopt_t) {
	return true;
}
template<typename T>
bool operator>=(dp::nullopt_t, const dp::optional<T>& rhs) {
	return !rhs.has_value();
}

template<typename T, typename U>
bool operator==(const dp::optional<T>& lhs, const U& rhs) {
	return (lhs.has_value()) ? *lhs == rhs : false;
}
template<typename T, typename U>
bool operator==(const U& lhs, const dp::optional<T>& rhs) {
	return (rhs.has_value()) ? *rhs == lhs : false;
}
template<typename T, typename U>
bool operator!=(const dp::optional<T>& lhs, const U& rhs) {
	return !(lhs == rhs);
}
template<typename T, typename U>
bool operator!=(const U& lhs, const dp::optional<T>& rhs) {
	return !(lhs == rhs);
}
template<typename T, typename U>
bool operator<(const dp::optional<T>& lhs, const U& rhs) {
	return (lhs.has_value()) ? *lhs < rhs : true;
}
template<typename T, typename U>
bool operator<(const U& lhs, const dp::optional<T>& rhs) {
	return(rhs.has_value()) ? lhs < *rhs : false;
}
template<typename T, typename U>
bool operator<=(const dp::optional<T>& lhs, const U& rhs) {
	return (lhs == rhs) || (lhs < rhs);
}
template<typename T, typename U>
bool operator<=(const U& lhs, const dp::optional<T>& rhs) {
	return (lhs == rhs) || (lhs < rhs);
}
template<typename T, typename U>
bool operator>(const dp::optional<T>& lhs, const U& rhs) {
	return !(lhs <= rhs);
}
template<typename T, typename U>
bool operator>(const U& lhs, const dp::optional<T>& rhs) {
	return !(lhs <= rhs);
}
template<typename T, typename U>
bool operator>=(const dp::optional<T>& lhs, const U& rhs) {
	return !(lhs < rhs);
}
template<typename T, typename U>
bool operator>=(const U& lhs, const dp::optional<T>& rhs) {
	return !(lhs < rhs);
}

}

#endif