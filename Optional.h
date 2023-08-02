#ifndef CPP03_OPTIONAL
#define CPP03_OPTIONAL

#include <new>
#include <cassert>

/*
*   An optional is a class which either contains a value or it does not. In the case it does not, it does not go to the effort of
*   constructing the held object.
*   The difference between this class and something like a std::pair<T,bool> is that this class maintains a well-defined state
*   in both cases. If it does not contain a value, the bool is false and there really isn't a meaningful value there. If true,
*   there always is a well-defined value. Plus, as mentioned, it defers actually constructing its contained type until that data
*   is needed, if ever. This in turn removes restrictions imposed by the other approaches - the held type no longer needs to be
*   default-constructible and expensive-to-construct types are only created when they are actually needed.
*   Obligatory comment that sizeof(Optional) = sizeof(T) + sizeof(bool). Even if we don't ever construct the contained type,
*   there's no escaping that space for it needs to be reserved on the stack. That's just how C++ works
*
*   Implementation decisions:
*   - We can't use alignment operators to police memory as they are a C++11 feature
*   - We can't use a union containing T directly, as pre-C++11 unions were not permitted to contain
*     types with nontrivial member functions
*
*   Implementation notes:
*   - Yup, the lifetimes are technically UB. Yes I'll fix it if I can get a working std::launder analogue (if possible)
*	  and a good enough read of the ancient C++03 standard text to make sure it matters.
*   - Yes I wish we were in C++17 too and have questioned my own sanity when writing this more times than you
*     questioned it while reading.
*
*   NB: DO NOT USE UnicodeString or the other Delphi String types. Their allocation is messed up under the hood in ways
*   which no reasonable person would code. It's tied to internal processes written in inline asm which cannot reasonably
*   be touched here. Use a more sensible string type like std::wstring instead.
*   They are effectively disabled by a nonexistent specialisation near the bottom of the file
*/

//Definitely swap out for std::optional at the earliest opportunity.


//A token "tag" to represent an empty optional
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


//Don't use these. They're left undefined for good reason.
//Blame Embarcadero not allocating and deallocating memory with any rhyme or reason.
//You will segfault, or at least as close as the Embarcadero runtime libraries will let you get.
template<>
class Optional<System::UnicodeString>;
template<>
class Optional<System::AnsiString>;

#endif