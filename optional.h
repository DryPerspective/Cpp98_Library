#ifndef CPP98_OPTIONAL
#define CPP98_OPTIONAL

#include <new>
#include <cassert>

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
		assert(m_HasValue && "Attempt to access uninitialized stored object");
		return *reinterpret_cast<T*>(m_Storage);
	}

	inline const T& storedObject() const{
        assert(m_HasValue && "Attempt to access uninitialized stored object");
		return *reinterpret_cast<const T*>(m_Storage);
	}

public:

    optional() : m_HasValue(false) {}

	optional(const T& Value) : m_HasValue(true){
		new (m_Storage) T(Value);
	}

	optional(nullopt_t) : m_HasValue(false) {}

	//We need to ensure a deep copy is made by calling on the copy semantics of type T, rather than wholesale copying
	//a collection of bits which represent T.
	optional(const optional& in) : m_HasValue(in.m_HasValue) {
		//Not initialized so can't assign it as a T.
		if(m_HasValue) new (m_Storage) T(in.storedObject());
	}

    ~optional()
	{
		this->reset();
	}

	optional& operator=(const T& in) {
		if (m_HasValue) storedObject() = in;
		else{
			new (m_Storage) T(in);
			m_HasValue = true;
		}
		return *this;
	}

	optional& operator=(const optional& in){
	//Note that optional::swap's exception specification depends on both swapping and copying.
	//Copy-and-swap should still maintain the strong exception guarantee here. Assuming no abnormal exception behaviour
	//which would also trip up a usual copy-and-swap (e.g. throwing destructor), the latest an exception can be thrown is 
	//in the first operation within swap(), at which point the original state in both cases will not have been modified
		  optional copy(in);
		  this->swap(copy);
		  return *this;
	}

	optional& operator=(nullopt_t){
		this->reset();
		return *this;
	}

	void reset(){
 		if(m_HasValue){
			this->storedObject().~T();
			m_HasValue = false;
		}
	}

	void swap(optional& other){
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
		if(m_HasValue && other.m_HasValue){

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
	operator bool() const { return has_value(); }

	T& value(){
		return storedObject();
	}

	const T& value() const{
		return storedObject();
	}

	//Alas no move semantics/forwarding references
	template<typename Other>
	T value_or(const Other& other){
		if(has_value()) return storedObject();
		else return other;
	}


    //Comparison operators
	//Friend members pattern to allow implicit conversion of solid values with optional ones
	template<typename U>
	friend bool operator==(const optional<U>& lhs, const optional<U>& rhs) {
		if (lhs && rhs) {
			return *lhs == *rhs;
		}
		else return !lhs && !rhs;
	}

	template<typename U>
	friend bool operator!=(const optional<U>& lhs, const optional<U>& rhs) {
		return !(lhs == rhs);
	}

	template<typename U>
	friend bool operator<(const optional<U>& lhs, const optional<U>& rhs) {
		if (lhs && rhs) {
			return *lhs < *rhs;
		}
		return !lhs && rhs;
	}

	template<typename U>
	friend bool operator<=(const optional<U>& lhs, const optional<U>& rhs) {
		return lhs < rhs || lhs == rhs;
	}

	template<typename U>
	friend bool operator>(const optional<U>& lhs, const optional<U>& rhs) {
		return !(lhs <= rhs);
	}

	template<typename U>
	friend bool operator>=(const optional<U>& lhs, const optional<U>& rhs) {
		return !(lhs < rhs);
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

}

#endif
