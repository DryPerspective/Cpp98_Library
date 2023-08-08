#ifndef CPP98_OPTIONAL
#define CPP98_OPTIONAL

#include <new>
#include <cassert>

/*
*	An analogue of std::optional which should work as far back as C++03.
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
struct NullOpt{};
//Can't to the old extern instance trick - the C++Builder linker says no

template<typename T>
class Optional{

	typedef T contained_type; //Typedef to avoid two-phase lookup issues

	//Union trick to delay initialization
    union {
		unsigned char m_Storage[sizeof(contained_type)];    //The data representing the object.
		long double m_phony_max_align;          			//Hacky analogue of std::max_align_t
	};
	bool      m_HasValue;


public:

    Optional() : m_HasValue(false) {}

	Optional(const T& Value) : m_HasValue(true){
		new (m_Storage) T(Value);
	}

    Optional(NullOpt) : m_HasValue(false) {}

    ~Optional()
	{
		if (m_HasValue) reinterpret_cast<T*>(m_Storage)->~T();
	}

	Optional& operator=(const T& in) {
		if (m_HasValue) *reinterpret_cast<T*>(m_Storage) = in;
		else{
			new (m_Storage) T(in);
			m_HasValue = true;
		}
		return *this;
	}

	Optional& operator=(NullOpt){
		this->reset();
		return *this;
	}

	void reset(){
 		if(m_HasValue){
			reinterpret_cast<T*>(m_Storage)->~T();
			m_HasValue = false;
		}
	}

	void swap(Optional& rhs){
		//Two step swap
		using std::swap;
		swap(this->m_HasValue, rhs.m_HasValue);
		swap(*reinterpret_cast<T*>(m_Storage),*reinterpret_cast<T*>(rhs.m_Storage));

	}

	const T& operator*() const{
		assert(m_HasValue && "Empty optional dereferenced");
		return *reinterpret_cast<const T*>(m_Storage);
    }

	T& operator*(){
		assert(m_HasValue && "Empty optional dereferenced");
		return *reinterpret_cast<T*>(m_Storage);
	}

	const T* operator->() const{
		assert(m_HasValue && "Empty optional dereferenced");
		return reinterpret_cast<const T*>(m_Storage);
	}

	T* operator->(){
		assert(m_HasValue && "Empty optional dereferenced");
		return reinterpret_cast<T*>(m_Storage);
	}

    bool has_value() const { return m_HasValue; }
    operator bool() const { return has_value(); }
};

//Left undefined to prevent use, for obvious reasons
template<>
class Optional<NullOpt>;

template<typename T>
void swap(Optional<T>& lhs, Optional<T>& rhs){
    return lhs.swap(rhs);
}

template<typename T>
Optional<T> make_Optional() {
	return Optional<T>();
}

template<typename T, typename U>
Optional<T> make_Optional(const U& in) {
	return Optional<T>(in);
}


//Comparison operators
template<typename T>
bool operator==(const Optional<T>& lhs, const Optional<T>& rhs){
	if(lhs && rhs){
		return *lhs == *rhs;
	}
    else return !lhs && !rhs;
}

template<typename T>
bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs){
	return !(lhs == rhs);
}

template<typename T>
bool operator<(const Optional<T>& lhs, const Optional<T>& rhs){
	if(lhs && rhs){
		return *lhs < *rhs;
	}
	return !lhs && rhs;
}

template<typename T>
bool operator<=(const Optional<T>& lhs, const Optional<T>& rhs){
	return lhs < rhs || lhs == rhs;
}

template<typename T>
bool operator>(const Optional<T>& lhs, const Optional<T>& rhs){
	return !(lhs <= rhs);
}

template<typename T>
bool operator>=(const Optional<T>& lhs, const Optional<T>& rhs){
	return !(lhs < rhs);
}

}

#endif