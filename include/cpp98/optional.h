#ifndef DP_CPP98_OPTIONAL
#define DP_CPP98_OPTIONAL

#include <new>
#include <exception>
#include <algorithm> //For std::swap, which lived in <algorithm> before C++11

#include "bits/version_defs.h"

#include "bits/unbound_storage.h"

/*
*	An analogue of std::optional which should work as far back as C++98.
*
*   Implementation decisions:
*   - We can't use alignment operators to police memory as they are a C++11 feature
*   - We can't use a union containing T directly, as pre-C++11 unions were not permitted to contain
*     types with nontrivial member functions
*
*/

//It's better to use std::optional and you should be able to swap this out for it when you get to C++17

namespace dp{

//A token "tag" to represent an empty optional
struct nullopt_t{};
//Can't to the old extern instance trick - the C++Builder linker says no
static const nullopt_t nullopt = {};

#ifndef DP_BORLAND_EXCEPTIONS
//Bad optional access exception
struct bad_optional_access : std::exception {
	bad_optional_access() {}
	virtual ~bad_optional_access() throw() {}
	virtual const char* what() const throw() {
		return "Bad optional access";
	}
};
#else
struct bad_optional_access : System::Sysutils::Exception {
	bad_optional_access() : System::Sysutils::Exception(L"Bad optional access") {}
};
#endif


	template<typename T>
	class optional {

	public:
		//Required typedef and two-phase lookup fixer
		typedef T value_type;


		optional() : m_storage() {}
		optional(dp::nullopt_t) : m_storage() {}

		optional(const optional& other) : m_storage() {
			if (other.has_value()) m_storage.template construct<value_type>(*other);
		}

		template<typename U>
		optional(const optional<U>& other) : m_storage() {
			if (other.has_value()) m_storage.template construct<value_type>(*other);
		}

		template<typename U>
		optional(const U& value) : m_storage() {
			m_storage.template construct<value_type>(value);
		}

		~optional() {
			m_storage.template reset<value_type>();
		}

		optional& operator=(dp::nullopt_t) {
			m_storage.template reset<value_type>();
			return *this;
		}
		optional& operator=(const optional& other) {
			optional copy(other);
			this->swap(copy);
			return *this;
		}
		template<typename U>
		optional& operator=(const U& in) {
			m_storage.template assign<value_type>(in);
			return *this;
		}
		template<typename U>
		optional& operator=(const dp::optional<U>& in) {
			optional copy(in);
			this->swap(copy);
			return *this;
		}

		void swap(optional& other) {
			m_storage.template swap<value_type>(other.m_storage);
		}

		void reset() {
			m_storage.template reset<T>();
		}

		value_type& operator*() {
			return get();
		}
		const value_type& operator*() const {
			return get();
		}

		value_type* operator->() {
			return &get();
		}
		const value_type* operator->() const {
			return &get();
		}

		bool has_value() const {
			return m_storage.has_value();
		}

		operator bool() const {
			return has_value();
		}

		value_type& value() {
			if (!has_value()) throw dp::bad_optional_access();
			return get();
		}
		const value_type& value() const {
			if (!has_value()) throw dp::bad_optional_access();
			return get();
		}

		template<typename U>
		value_type value_or(const U& in) const {
			if (!has_value()) return in;
			return get();
		}

	private:

		typename dp::unbound_storage<sizeof(value_type)> m_storage;

		inline value_type& get() {
			return m_storage.template get<value_type>();
		}
		inline const value_type& get() const {
			return m_storage.template get<value_type>();
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