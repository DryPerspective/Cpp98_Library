#ifndef CPP98_TYPE_TRAITS_NONSTANDARD
#define CPP98_TYPE_TRAITS_NONSTANDARD

#include "type_traits.h"

/*
*	A collection of useful, general-purpose type traits which power one or more other features in the library; but which are not included in the modern type_traits header
*	
*/

namespace dp {

    /*
    *  We cannot create a general purpose std::decay because that requires an is_function trait and that be complete without variadic templates (or compiler intrinsics)
    *  But the ability to decay a C-array to a pointer is still useful
    */
    namespace detail {
        template<typename T, bool = dp::is_array<T>::value>
        struct decay_array {
            typedef typename dp::add_pointer<typename dp::remove_extent<T>::type>::type type;
        };
        template<typename T>
        struct decay_array<T, false> {};
    }

    template<typename T>
    struct decay_array : detail::decay_array<T> {};


    /*
    *  A means to examine and extract the nth template type for a particular template
    *  This is done much more elegantly (and completely) in C++11 and above with variadics and tuples
    *  But in this world I'm not beholden to meeting standard guarantees so something which will work for all but the most
    *  pathological examples should work.
    *  Frankly, who writes a template with more than ten parameters?
    */
    template<typename T>
    struct param_types {
        typedef T template_type;
    };
    template<template<typename> class templ, typename T0>
    struct param_types<templ<T0> > {
        typedef templ<T0>   template_type;
        typedef T0          first_param_type;
    };
    template<template<typename, typename> class templ, typename T0, typename T1>
    struct param_types<templ<T0, T1> > {
        typedef templ<T0, T1>   template_type;
        typedef T0              first_param_type;
        typedef T1              second_param_type;
    };
    template<template<typename, typename, typename> class templ, typename T0, typename T1, typename T2>
    struct param_types<templ<T0, T1, T2> > {
        typedef templ<T0, T1, T2>   template_type;
        typedef T0                  first_param_type;
        typedef T1                  second_param_type;
        typedef T2                  third_param_type;
    };
    template<template<typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3>
    struct param_types<templ<T0, T1, T2, T3> > {
        typedef templ<T0, T1, T2, T3>   template_type;
        typedef T0                      first_param_type;
        typedef T1                      second_param_type;
        typedef T2                      third_param_type;
        typedef T3                      fourth_param_type;
    };
    template<template<typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4>
    struct param_types<templ<T0, T1, T2, T3, T4> > {
        typedef templ<T0, T1, T2, T3, T4>   template_type;
        typedef T0                          first_param_type;
        typedef T1                          second_param_type;
        typedef T2                          third_param_type;
        typedef T3                          fourth_param_type;
        typedef T4                          fifth_param_type;
    };
    template<template<typename, typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
    struct param_types<templ<T0, T1, T2, T3, T4, T5> > {
        typedef templ<T0, T1, T2, T3, T4, T5>   template_type;
        typedef T0                              first_param_type;
        typedef T1                              second_param_type;
        typedef T2                              third_param_type;
        typedef T3                              fourth_param_type;
        typedef T4                              fifth_param_type;
        typedef T5                              sixth_param_type;
    };
    template<template<typename, typename, typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    struct param_types<templ<T0, T1, T2, T3, T4, T5, T6> > {
        typedef templ<T0, T1, T2, T3, T4, T5, T6>   template_type;
        typedef T0                                  first_param_type;
        typedef T1                                  second_param_type;
        typedef T2                                  third_param_type;
        typedef T3                                  fourth_param_type;
        typedef T4                                  fifth_param_type;
        typedef T5                                  sixth_param_type;
        typedef T6                                  seventh_param_type;
    };
    template<template<typename, typename, typename, typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    struct param_types<templ<T0, T1, T2, T3, T4, T5, T6, T7> > {
        typedef templ<T0, T1, T2, T3, T4, T5, T6, T7>   template_type;
        typedef T0                                      first_param_type;
        typedef T1                                      second_param_type;
        typedef T2                                      third_param_type;
        typedef T3                                      fourth_param_type;
        typedef T4                                      fifth_param_type;
        typedef T5                                      sixth_param_type;
        typedef T6                                      seventh_param_type;
        typedef T7                                      eighth_param_type;
    };

    // Specialization for Nine Template Parameters
    template<template<typename, typename, typename, typename, typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    struct param_types<templ<T0, T1, T2, T3, T4, T5, T6, T7, T8> > {
        typedef templ<T0, T1, T2, T3, T4, T5, T6, T7, T8>   template_type;
        typedef T0                                          first_param_type;
        typedef T1                                          second_param_type;
        typedef T2                                          third_param_type;
        typedef T3                                          fourth_param_type;
        typedef T4                                          fifth_param_type;
        typedef T5                                          sixth_param_type;
        typedef T6                                          seventh_param_type;
        typedef T7                                          eighth_param_type;
        typedef T8                                          ninth_param_type;
    };

    // Specialization for Ten Template Parameters
    template<template<typename, typename, typename, typename, typename, typename, typename, typename, typename, typename> class templ, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    struct param_types<templ<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> > {
        typedef templ<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>   template_type;
        typedef T0                                              first_param_type;
        typedef T1                                              second_param_type;
        typedef T2                                              third_param_type;
        typedef T3                                              fourth_param_type;
        typedef T4                                              fifth_param_type;
        typedef T5                                              sixth_param_type;
        typedef T6                                              seventh_param_type;
        typedef T7                                              eighth_param_type;
        typedef T8                                              ninth_param_type;
        typedef T9                                              tenth_param_type;
    };
    /*
    *  Special case for std::array analogue
    */
    template<template<typename, std::size_t> class templ, typename T, std::size_t N>
    struct param_types<templ<T, N> > {
        typedef templ<T,N>  template_type;
        typedef T           first_param_type;
        typedef std::size_t size_type;
    };



}


#endif
