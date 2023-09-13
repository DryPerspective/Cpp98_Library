#ifndef CPP98_NUMERIC
#define CPP98_NUMERIC

#include <cmath>

#include "type_traits.h"

namespace dp {

	/*
	*  A quick an easy recreation of a handful of numeric functions
	*  No transform, transform_reduce, etc at present, as these were added to support parallelisation
	*  And C++98 has no formal concurrency features.
	*/

template<typename ForwardIt, typename T>
void iota(ForwardIt first, ForwardIt second, T value) {
	while (first != last) {
		*first++ = value;
		++value;
	}
}



/*
*  Time for some ugly template magic because we don't have common_type
*  Because signedness is a curse and std::abs is ambiguous for unsigned types, we need to filter for the four possibilities
*/
namespace detail {
	template<typename T, typename U>
	typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type gcd_impl(T t, U u) {
		if (t == 0 && u == 0) return 0;

		typedef typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type result_type;
		while (u != 0) {
			result_type temp = u;
			u = t % u;
			t = temp;
		}
		return t;
	}

	template<typename T, typename U>
	typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type lcm_impl(T t, U u) {
		if (t == 0 || u == 0) return 0;
		return (t * u) / gcd(t, u);
	}
}

//Both unsigned
template<typename T, typename U>
typename dp::enable_if <dp::is_integral<T>::value && dp::is_integral<U>::value 
					 && dp::is_unsigned<T>::value && dp::is_unsigned<U>::value, 
		typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type>::type gcd(T t, U u) {
	return detail::gcd_impl(t, u);
}
template<typename T, typename U>
typename dp::enable_if <dp::is_integral<T>::value&& dp::is_integral<U>::value
					 && dp::is_unsigned<T>::value&& dp::is_unsigned<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type>::type lcm(T t, U u) {
	return detail::lcm_impl(t, u);
}

//T unsigned, U signed
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						&& dp::is_unsigned<T>::value&& dp::is_signed<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), T, typename dp::make_unsigned<U>::type>::type>::type gcd(T t, U u) {
	return detail::gcd_impl(t, std::abs(u));
}
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						&& dp::is_unsigned<T>::value&& dp::is_signed<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), T, typename dp::make_unsigned<U>::type>::type>::type lcm(T t, U u) {
	return detail::lcm_impl(t, std::abs(u));
}

//T signed, U unsigned
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						&& dp::is_signed<T>::value&& dp::is_unsigned<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), typename make_unsigned<T>::type, U>::type>::type gcd(T t, U u) {
	return detail::gcd_impl(std::abs(t), u);
}
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						&& dp::is_signed<T>::value&& dp::is_unsigned<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), typename make_unsigned<T>::type, U>::type>::type lcm(T t, U u) {
	return detail::lcm_impl(std::abs(t), u);
}

//Both signed
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						  && dp::is_signed<T>::value&& dp::is_signed<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), typename make_unsigned<T>::type, typename make_unsigned<U>::type>::type>::type gcd(T t, U u) {
	return detail::gcd_impl(std::abs(t), std::abs(u));
}
//Both signed
template<typename T, typename U>
typename dp::enable_if < dp::is_integral<T>::value&& dp::is_integral<U>::value
						  && dp::is_signed<T>::value&& dp::is_signed<U>::value,
	typename dp::conditional<sizeof(T) >= sizeof(U), typename make_unsigned<T>::type, typename make_unsigned<U>::type>::type>::type lcm(T t, U u) {
	return detail::lcm_impl(std::abs(t), std::abs(u));
}

template<typename T>
T midpoint(T a, T b) {
	return (a + b) / 2;
}

template<typename T>
T* midpoint(T* a, T* b) {
	return a + (b - a) / 2;
}


}

#endif
