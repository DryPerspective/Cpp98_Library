#ifndef DP_CPP98_RATIO
#define DP_CPP98_RATIO

#include <cctype>

#include "cpp98/static_assert.h"    //To assert no zero denominators
#include "cpp98/type_traits.h"

namespace dp{

namespace detail{
    //First we need a few metafunctions to get gcd, sign, and abs 
    //at compile time

    template<std::ptrdiff_t Temp, std::ptrdiff_t T, std::ptrdiff_t U>
    struct comp_gcd_impl{
        static const std::ptrdiff_t value = comp_gcd_impl<T,U,T%U>::value; 
    };

    template<std::ptrdiff_t Temp, std::ptrdiff_t T>
    struct comp_gcd_impl<Temp, T, 0>{
        static const std::ptrdiff_t value = T;
    };

        template<std::ptrdiff_t T, std::ptrdiff_t U>
    struct comp_gcd : detail::comp_gcd_impl<T, T, U> {};

    template<std::ptrdiff_t Temp>
    struct comp_gcd_impl<Temp,0,0>{
        static const std::ptrdiff_t value = 0;
    };

    template<std::ptrdiff_t val>
    struct abs{
        static const std::ptrdiff_t value = val >= 0 ? val : -val;
    };

    template<std::ptrdiff_t val>
    struct sign{
        static const std::ptrdiff_t value = val >= 0 ? 1 : -1;
    };
}

template<std::ptrdiff_t Num, std::ptrdiff_t Denom = 1>
struct ratio : dp::static_assert_98<Denom != 0>
{
    static const std::ptrdiff_t num = detail::sign<Denom>::value * Num/detail::comp_gcd<Num,Denom>::value;
    static const std::ptrdiff_t den = detail::abs<Denom/detail::comp_gcd<Num,Denom>::value>::value;
    typedef dp::ratio<num,den> type;
};

//Alas no using aliases, so ratio math comes with extra clunky syntax
//We mitigate this a little by adding valid num and den members to match
//the user end interface of a modern std::ratio_sum.
template<typename R1, typename R2>
struct ratio_add{
    static const std::ptrdiff_t num = R1::num * R2::den + R2::num * R1::den;
    static const std::ptrdiff_t den = R1::den * R2::den;
    typedef dp::ratio<num, den> type;
};

template<typename R1, typename R2>
struct ratio_subtract{
    static const std::ptrdiff_t num = R1::num * R2::den - R2::num * R1::den;
    static const std::ptrdiff_t den = R1::den * R2::den;
    typedef dp::ratio<num,den> type;
};

template<typename R1, typename R2>
struct ratio_multiply{
    static const std::ptrdiff_t num = R1::num * R2::num;
    static const std::ptrdiff_t den = R1::den * R2::den;
    typedef dp::ratio<num,den> type;
};

template<typename R1, typename R2>
struct ratio_divide{
    static const std::ptrdiff_t num = R1::num * R2::den;
    static const std::ptrdiff_t den = R1::den * R2::num;
    typedef dp::ratio<num,den> type;
};

template<typename R1, typename R2>
struct ratio_equal : dp::integral_constant<bool, R1::num == R2::num && R1::den == R2::den> {};

template<typename R1, typename R2>
struct ratio_not_equal : dp::integral_constant<bool, !dp::ratio_equal<R1,R2>::value> {};

template<typename R1, typename R2>
struct ratio_less : dp::integral_constant<bool, (R1::num * R2::den) < (R2::num * R1::den)> {};

template<typename R1, typename R2>
struct ratio_less_equal : dp::integral_constant<bool, dp::ratio_less<R1,R2>::value || dp::ratio_equal<R1,R2>::value> {};

template<typename R1, typename R2>
struct ratio_greater : dp::integral_constant<bool, dp::ratio_less<R2,R1>::value> {};

template<typename R1, typename R2>
struct ratio_greater_equal : dp::integral_constant<bool, dp::ratio_less_equal<R2,R1>::value> {};


typedef dp::ratio<1,       1000000000000000000> atto;   
typedef dp::ratio<1,          1000000000000000> femto;  
typedef dp::ratio<1,             1000000000000> pico;   
typedef dp::ratio<1,                1000000000> nano;   
typedef dp::ratio<1,                   1000000> micro;  
typedef dp::ratio<1,                      1000> milli;  
typedef dp::ratio<1,                       100> centi;  
typedef dp::ratio<1,                        10> deci;   
typedef dp::ratio<                       10, 1> deca;   
typedef dp::ratio<                      100, 1> hecto;  
typedef dp::ratio<                     1000, 1> kilo;   
typedef dp::ratio<                  1000000, 1> mega;   
typedef dp::ratio<               1000000000, 1> giga;   
typedef dp::ratio<            1000000000000, 1> tera;   
typedef dp::ratio<         1000000000000000, 1> peta;   
typedef dp::ratio<      1000000000000000000, 1> exa;   

}

#endif