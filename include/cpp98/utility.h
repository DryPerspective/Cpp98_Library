#ifndef DP_CPP98_UTILITY
#define DP_CPP98_UTILITY

#include <limits>
#include <utility>

#include "cpp98/type_traits.h"


namespace dp {

	//Note, copy semantics as we're preC++11
	template<typename T, typename U>
	T exchange(T& obj, const U& new_value) {
		T old_value = obj;
		obj = new_value;
		return old_value;
	}


	template<typename T>
	typename dp::add_const<T>::type& as_const(T& in) {
		return in;
	}

	//No definition needed. This is an unevaluated-context function
	template<typename T>
	T& declval();

#ifdef DP_CPP20_INTERFACE
	/*
	*  Non-converting comparison functions. Yippee.
	*  I don't have constexpr if here, so unfortunately we need to do this the hard way
	*/
	template<typename T, typename U>
	typename dp::enable_if<dp::is_signed<T>::value == dp::is_signed<U>::value, bool>::type cmp_equal(T t, U u) {
		return t == u;
	}
	template<typename T, typename U>
	typename dp::enable_if<dp::is_signed<T>::value && !is_signed<U>::value, bool>::type cmp_equal(T t, U u) {
		return t >= 0 && typename dp::make_unsigned<T>::type(t) == u;
	}
	template<typename T, typename U>
	typename dp::enable_if<!dp::is_signed<T>::value&& is_signed<U>::value, bool>::type cmp_equal(T t, U u) {
		return u >= 0 && typename dp::make_unsigned<U>::type(u) == t;
	}

	template<typename T, typename U>
	typename dp::enable_if<dp::is_signed<T>::value == dp::is_signed<U>::value, bool>::type cmp_less(T t, U u) {
		return t < u;
	}
	template<typename T, typename U>
	typename dp::enable_if<dp::is_signed<T>::value && !is_signed<U>::value, bool>::type cmp_less(T t, U u) {
		return t < 0 || typename dp::make_unsigned<T>::type(t) < u;
	}
	template<typename T, typename U>
	typename dp::enable_if<!dp::is_signed<T>::value&& is_signed<U>::value, bool>::type cmp_less(T t, U u) {
		return u >= 0 && t < typename dp::make_unsigned<U>::type(u);
	}

	template<typename T, typename U>
	bool cmp_not_equal(T t, U u) {
		return !dp::cmp_equal(t, u);
	}

	template<typename T, typename U>
	bool cmp_greater(T t, U u) {
		return dp::cmp_less(u, t);
	}

	template<typename T, typename U>
	bool cmp_less_equal(T t, U u) {
		return !dp::cmp_less(u, t);
	}

	template<typename T, typename U>
	bool cmp_greater_equal(T t, U u) {
		return !dp::cmp_less(t, u);
	}

	template<typename Range, typename T>
	bool in_range(T t) {
		return dp::cmp_greater_equal(t, std::numeric_limits<Range>::min()) &&
			dp::cmp_less_equal(t, std::numeric_limits<Range>::max());
	}
#endif

}

#endif