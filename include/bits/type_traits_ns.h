#ifndef DP_CPP98_TYPE_TRAITS_NONSTANDARD
#define DP_CPP98_TYPE_TRAITS_NONSTANDARD

#include "bits/version_defs.h"

#include "cpp98/type_traits.h"

/*
*	A collection of useful, general-purpose type traits which power one or more other features in the library; but which are not included in the modern type_traits header
*	
*/

namespace dp {


    template<typename T, typename U>
    struct type_sizes {
        typedef typename dp::conditional<sizeof(T) >= sizeof(U), T, U>::type larger_type;
        typedef typename dp::conditional<sizeof(T) >= sizeof(U), U, T>::type smaller_type;
        static const std::size_t larger_size = sizeof(larger_type);
        static const std::size_t smaller_size = sizeof(smaller_type);
    };

    template<typename T, typename U>
    struct is_qualification_conversion {
        static const bool value =
            dp::is_same<T, U>::value ||                                                                     //Same qualification
            dp::is_same<T, typename dp::remove_cv<U>::type>::value ||                                       //Unqualified to c/v/cv
            (dp::is_const<T>::value && dp::is_same<T, typename dp::remove_volatile<U>::type>::value) ||     //Const to cv
                (dp::is_volatile<T>::value && dp::is_same<T, typename dp::remove_const<U>::type>::value);   //Volatile to cv
    };

    namespace detail{
#ifndef DP_BORLAND
        //is_convertible relies on a SFINAE trick which borland never implemented.
        template<typename Iter, typename Tag, bool = dp::is_convertible<typename std::iterator_traits<Iter>::iterator_category, Tag>::value >
        struct is_iter_category : dp::false_type {};

        template<typename Iter, typename Tag>
        struct is_iter_category<Iter, Tag, true> : dp::true_type {};
#else
        template<typename Iter, typename Tag>
        struct is_iter_category : dp::false_type {};

        template<typename Iter>
        struct is_iter_category<Iter, std::input_iterator_tag> {
            static const bool value = dp::is_same<typename std::iterator_traits<Iter>::iterator_category, std::input_iterator_tag>::value;
        };

        template<typename Iter>
        struct is_iter_category<Iter, std::output_iterator_tag> {
            static const bool value = dp::is_same<typename std::iterator_traits<Iter>::iterator_category, std::input_iterator_tag>::value;
        };

        template<typename Iter>
        struct is_iter_category<Iter, std::random_access_iterator_tag> {
            static const bool value = dp::is_same<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>::value;
        };

        template<typename Iter>
        struct is_iter_category<Iter, std::bidirectional_iterator_tag> {
            static const bool value = dp::detail::is_iter_category<Iter, std::random_access_iterator_tag>::value || dp::is_same<typename std::iterator_traits<Iter>::iterator_category, std::bidirectional_iterator_tag>::value;
        };

        template<typename Iter>
        struct is_iter_category<Iter, std::forward_iterator_tag> {
            static const bool value = dp::detail::is_iter_category<Iter, std::bidirectional_iterator_tag>::value || dp::is_same<typename std::iterator_traits<Iter>::iterator_category, std::bidirectional_iterator_tag>::value;
        };



#endif
    }

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

    //Whether a type is a valid candidate for "value semantics". i.e. it is not an array, easily copied, and copies produce equivalent types
    template<typename T>
    struct is_value_type {
        static const bool value = !dp::is_array<T>::value && !dp::is_pointer<T>::value && !dp::is_reference<T>::value
#ifndef DP_BORLAND
            && dp::is_copy_constructible<T>::value && dp::is_copy_assignable<T>::value
#endif
            ;
    };
    

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
