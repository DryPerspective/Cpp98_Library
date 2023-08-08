#ifndef CPP03_OPTIONAL
#define CPP03_OPTIONAL

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
//Feel free to create a globally scoped constant of this if you want less awkward syntax
struct NullOpt{};


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
		if(m_HasValue){
			reinterpret_cast<T*>(m_Storage)->~T();
			m_HasValue = false;
		}
		return *this;
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


}

#endif