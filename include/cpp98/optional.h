#ifndef DP_CPP98_OPTIONAL
#define DP_CPP98_OPTIONAL

#include <new>
#include <exception>
#include <algorithm> //For std::swap, which lived in <algorithm> before C++11

#include "bits/version_defs.h"

#include "bits/optional_expected_base.h"

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

/*
*  dp::optional and dp::expected both use an almost identical mechanism to store (or not store) their values. As such, we DRY up our code
*  by having them share a base.
*/
template<typename T>
class optional : public dp::detail::opt_exp_base<T, sizeof(T)> {
	typedef typename dp::detail::opt_exp_base<T, sizeof(T)> Base;

public:

	//Public facing typedef. Less ugly than public-private-public switcheroo to use the other one
	typedef T value_type;

	optional() : Base(false) {}
	optional(nullopt_t) : Base(false) {}

	optional(const optional& other) : Base(other) {}

	template<typename U>
	optional(const optional<U>& other) : Base(other) {}

	template<typename U>
	optional(const U& in) : Base(in) {}

	~optional() {}

	#ifndef DP_BORLAND
	//No clude what the error here is, except that that Borland seems to think that Base isn't a base of this class.
	//Error message is just "detail::opt_exp_base<T, sizeof(T)> is not a base of optional<T>"
	using Base::operator=;
	#else
	template<typename U>
	optional& operator=(const U& in) {
		if (Base::m_HasValue) {
			using std::swap;
			T copy(in);
			swap(this->storedObject(), copy);
		}
		else {
			new (Base::m_Storage) T(in);
			Base::m_HasValue = true;
		}
		return *this;
	}

	template<typename U>
	optional& operator=(const optional<U>& in) {
		optional<T> copy(in);
		this->swap(copy);
		return *this;
	}
	#endif

	optional& operator=(dp::nullopt_t){
		this->reset();
		return *this;
	}

	//Not shared because of differing exceptions thrown.
	T& value() {
		if (!this->has_value()) throw dp::bad_optional_access();
		return this->storedObject();
	}

	const T& value() const {
		if (!this->has_value()) throw dp::bad_optional_access();
		return this->storedObject();
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