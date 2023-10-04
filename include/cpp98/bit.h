#ifndef DP_CPP98_BIT
#define DP_CPP98_BIT

#include <cstring>
#include <algorithm>
#include <climits>
#include <limits>

#include "bits/version_defs.h"

#include "cpp98/type_traits.h"
#include "cpp98/array.h"

namespace dp{

//No endian enum - it's implementation defined and even the provided examples
//don't cover everything (looking at you, Borland)

/*
*   Bit_cast.
*   Unlike std::bit_cast, my version is not blessed by the standard to be an exception to object
*   lifetime rules. Nothing to be done about that.
*   Also we substitue trivially copyable for copyable, because triviality is untestable.
*   Honestly, this is a bit safer than a reinterpret_cast, but don't be complacent with it
*/
template<typename To, typename From>
typename dp::enable_if<sizeof(To) == sizeof(From)
#ifndef DP_BORLAND
					 && dp::is_copy_constructible<To>::value &&
                      dp::is_copy_constructible<From>::value &&
					  dp::is_default_constructible<To>::value
#endif
					  ,To>::type
                      bit_cast(const From& from){
                            To dst;
                            std::memcpy(&dst, &from, sizeof(To));
                            return dst;
                      }

template<typename T>
typename dp::enable_if<dp::is_integral<T>::value, T>::type byteswap(T value){
    dp::array<unsigned char, sizeof(T)> bitrep = dp::bit_cast<dp::array<unsigned char, sizeof(T)> >(value);
    std::reverse(bitrep.begin(), bitrep.end());
    return dp::bit_cast<T>(bitrep);
}

namespace detail{
    template<typename T>
    struct bit_unsigned_int{
        static const bool value = dp::is_unsigned<T>::value && dp::is_integral<T>::value &&
                                 !dp::is_same<T, bool>::value && !dp::is_same<T, char>::value &&
                                 !dp::is_same<T, wchar_t>::value;
    };
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, bool>::type has_single_bit(T x){
    return x && !(x & (x - 1));
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, int>::type countl_zero(T x){
    unsigned int sign = sizeof(T) * CHAR_BIT;
    unsigned int mask = 1u << sign;
    for(unsigned int i = 0; i < sign; ++i, mask >>= 1){
        if(mask & x) return i - 1;
    }
    return sign; 
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, int>::type countl_one(T x){
    return dp::countl_zero(static_cast<T>(~x));
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, int>::type countr_zero(T x){
    unsigned int mask = 1u;
    for(unsigned int i = 0; i < CHAR_BIT * sizeof(T); ++i, mask <<= 1){
        if(mask & x) return i;
    }
    return CHAR_BIT * sizeof(T);
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value,int>::type countr_one(T x){
    return dp::countr_zero(static_cast<T>(~x));
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value,int>::type bit_width(T x){
    return std::numeric_limits<T>::digits - dp::countl_zero(x);
}

//Bit ceil - two versions because no if constexpr
template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value && sizeof(T) >= sizeof(unsigned int),
            T>::type bit_ceil(T x){
                if(x < 2) return 1;
                return static_cast<T>(1) << (std::numeric_limits<T>::digits - dp::countl_zero((T)(x - 1u)));
}
template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value && sizeof(T) < sizeof(unsigned int),
            T>::type bit_ceil(T x){
                if(x < 2) return 1;
            //Account for UB from promotion
                const unsigned int offset = std::numeric_limits<unsigned int>::digits  - std::numeric_limits<T>::digits;
                const unsigned int retval = 1u << (x + offset);
                return static_cast<T>(retval >> offset);
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, T>::type bit_floor(T x){
    return (x != 0) ? static_cast<T>(1) << (dp::bit_width(x) - 1) : 0;
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, T>::type rotr(T x, int s);
template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, T>::type rotl(T x, int s){
    T N = std::numeric_limits<T>::digits;
    T r = s % N;
    if(r == 0) return x;
    return (r > 0) ? ((x << r) | (x >> (N - r))) : dp::rotr(x, -r);
}
template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, T>::type rotr(T x, int s){
    T N = std::numeric_limits<T>::digits;
    T r = s % N;
    if(r == 0) return x;
    return (r > 0) ? ((x >> r) | (x << (N - r))) : dp::rotl(x, -r);
}

template<typename T>
typename dp::enable_if<detail::bit_unsigned_int<T>::value, int>::type popcount(T x){
    T mask = 1u;
    int count = 0;
    for(unsigned int i = 0; i < sizeof(T) * CHAR_BIT; ++i, mask <<= 1){
        if(x & mask) ++count;
    }
    return count;
} 


}

#endif