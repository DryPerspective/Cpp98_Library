#ifndef DP_CPP98_UNBOUND_STORAGE
#define DP_CPP98_UNBOUND_STORAGE

#include <new>
#include <algorithm> //For std::swap, which lived in <algorithm> before C++11

#include "bits/type_traits_ns.h"

namespace dp{

/*
*	An "unbound storage" type. Little more than a union with some member functions attached, but a core element of structures like optional,
*	where initialization of the data is to be deferred until needed, or multiple types need to fit in the same storage
*	Note that this class does NOT track the type currently held or tidy up after itself (adds overhead and RTTI). That's left to the user, and largely why
*	this is kept as an internal implementation detail.
*/
template<std::size_t N>
class unbound_storage {

	union {
		unsigned char m_storage[N];							//The data representing the object.
		long double m_phony_max_align;          			//Hacky analogue of std::max_align_t
	};
	bool      m_has_value;

	//Since we don't know or track our held type, we cannot allow simple copying.
	unbound_storage(const unbound_storage&);
	unbound_storage operator=(const unbound_storage&);

public:
	unbound_storage() : m_has_value(false) {};
	
	template<typename T>
	unbound_storage(const T& in) : m_has_value(true) {
		construct<T>(in);
	}


	~unbound_storage() {}

	template<typename T>
	static unbound_storage create(const T& in) {
		return unbound_storage(in);
	}

	template<typename T, typename U>
	static unbound_storage create(const U& in) {
		unbound_storage us;
		us.construct<T>(in);
		return us;
	}

	/*
	template<typename T>
	void construct(const T& in) {
		new (m_storage) T(in);
		m_has_value = true;
	}
	*/

	template<typename T, typename U>
	void construct(const U& in) {
		new (m_storage) T(in);
		m_has_value = true;
	}
	/*
	template<typename T>
	unbound_storage& assign(const T& in) {
		unbound_storage copy(in);
		this->swap<T>(copy);
		copy.reset<T>();
		return *this;
	}
	*/
	template<typename T, typename U>
	unbound_storage& assign(const U& in) {
		unbound_storage copy = unbound_storage::create<T>(in);
		this->swap<T>(copy);
		copy.reset<T>();
		return *this;
	}
	
	template<typename T>
	T& get() {
		return *reinterpret_cast<T*>(m_storage);
	}

	template<typename T>
	const T& get() const {
		return *reinterpret_cast<const T*>(m_storage);
	}

	template<typename T>
	void destroy() {
		get<T>().~T();
		m_has_value = false;
	}	

	template<typename T>
	void reset() {
		if(m_has_value)	destroy<T>();
	}

	template<typename T>
	void swap(unbound_storage& other) {
		using std::swap;
		if (has_value() && other.has_value()) {
			swap(this->get<T>(), other.get<T>());
		}
		else if (other.has_value()) {
			this->construct<T>(other.get<T>());
			other.destroy<T>();
		}
		else if (has_value()) {
			other.construct<T>(this->get<T>());
			this->destroy<T>();
		}
		swap(m_has_value, other.m_has_value);
	}


	bool has_value() const {
		return m_has_value;
	}

};



}


#endif