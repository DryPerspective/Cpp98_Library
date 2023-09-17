#ifndef DP_CPP98_TYPE_TRAITS
#define DP_CPP98_TYPE_TRAITS

/*
*   A C++98 compliant version of the type_traits header
*   Obviously not everything from type_traits can be backported. The _t and _v helper types and variables
*   are notably absent as they require language features not added until later versions of the standard
*
*   Additionally, the definitions or implementations of some traits require either compiler magic or language
*   features added in C++11 and up which cannot be replicated here. Common bases are is_class, is_function, 
*   is_union, etc and everything which builds on and requires those.
*   To be precise - is_union is compiler magic. is_class requires a check against !is_union.
*   implementing is_class without that check is possible, but will produce different results from
*   std::is_class in some circumstances - not good.
*/

#include <cctype>
#include <algorithm>

namespace dp{

/*
* FORWARD DECS
*/
template<typename, typename>
struct is_same;

template<typename>
struct remove_cv;

template<bool,typename = void>
struct enable_if;

template<typename>
struct type_identity;

template<typename>
struct remove_reference;

template<typename>
struct add_lvalue_reference;

template<typename>
struct is_const;

template<typename>
struct is_reference;

template<typename>
struct is_member_pointer;

namespace detail{
    template<typename T>
    typename add_lvalue_reference<T>::type declval();
}

/*
* BASE TYPES
*/
template<typename T, T v>
struct integral_constant{
    static const T value = v;
    typedef T value_type;
    typedef integral_constant type;
    operator value_type() const {return value;}
    value_type operator()() const{return value;}

};

typedef integral_constant<bool,true> true_type;
typedef integral_constant<bool,false> false_type;


template<bool b>
struct bool_constant : true_type {};

template<>
struct bool_constant<false> : false_type {};



/*
* TYPE CATEGORIES
*/
template<typename T>
struct is_void : is_same<void, typename remove_cv<T>::type> {};

template<typename T>
struct is_integral : integral_constant<bool,
                    is_same<typename remove_cv<T>::type, bool>::value ||
                    is_same<typename remove_cv<T>::type, signed char>::value ||
                    is_same<typename remove_cv<T>::type, unsigned char>::value ||
                    is_same<typename remove_cv<T>::type, signed short>::value ||
                    is_same<typename remove_cv<T>::type, unsigned short>::value ||
                    is_same<typename remove_cv<T>::type, signed int>::value ||
                    is_same<typename remove_cv<T>::type, unsigned int>::value ||
                    is_same<typename remove_cv<T>::type, signed long>::value ||
                    is_same<typename remove_cv<T>::type, unsigned long>::value ||
                    is_same<typename remove_cv<T>::type, wchar_t>::value> {};

template<typename T>
struct is_floating_point : integral_constant<bool,
                    is_same<typename remove_cv<T>::type, float>::value ||
                    is_same<typename remove_cv<T>::type, double>::value ||
                    is_same<typename remove_cv<T>::type, long double>::value> {};

template<typename T>
struct is_array : false_type {};
template<typename T>
struct is_array<T[]> : true_type {};
template<typename T, std::size_t N>
struct is_array<T[N]> : true_type {};

template<typename T>
struct is_function : dp::integral_constant<bool, !dp::is_const<const T>::value && !dp::is_reference<T>::value> {};

template<typename T>
struct is_pointer : false_type {};
template<typename T>
struct is_pointer<T*> : true_type {};
template<typename T>
struct is_pointer<T* const> : true_type {};
template<typename T>
struct is_pointer<T* volatile> : true_type {};
template<typename T>
struct is_pointer<T* const volatile> : true_type {};

template<typename T>
struct is_lvalue_reference : false_type {};
template<typename T>
struct is_lvalue_reference<T&> : true_type {};


namespace detail{
    template<typename T>
    struct is_memfun_ptr_impl : dp::false_type {};
    template<typename T, typename U>
    struct is_memfun_ptr_impl<T U::*> : dp::is_function<T> {};
}
template<typename T>
struct is_member_function_pointer : dp::detail::is_memfun_ptr_impl<typename dp::remove_cv<T>::type> {};

template<typename T>
struct is_member_object_pointer : dp::integral_constant<bool, dp::is_member_pointer<T>::value && ! dp::is_member_function_pointer<T>::value> {};

/*
* COMPOSITE TYPES
*/
template<typename T>
struct is_arithmetic : integral_constant<bool,
                        is_integral<T>::value ||
                        is_floating_point<T>::value> {};
            
template<typename T>
struct is_fundamental : integral_constant<bool,
                         is_arithmetic<T>::value ||
                         is_same<typename remove_cv<T>::type, void>::value> {};

template<typename T>
struct is_compound : integral_constant<bool, !is_fundamental<T>::value> {};

template<typename T>
struct is_reference : is_lvalue_reference<T> {};

namespace detail{
    template<typename T>
    struct is_member_pointer_impl : dp::false_type {};
    template<typename T, typename U>
    struct is_member_pointer_impl<T U::*> : dp::true_type{};
}
template<typename T>
struct is_member_pointer : dp::detail::is_member_pointer_impl<typename dp::remove_cv<T>::type> {};


/*
* TYPE PROPERTIES
*/
template<typename T>
struct is_const : false_type {};
template<typename T>
struct is_const<const T> : true_type {};

template<typename T>
struct is_volatile : false_type {};
template<typename T>
struct is_volatile<volatile T> : true_type {};


namespace detail
{
    template<typename T, bool = is_arithmetic<T>::value>
    struct is_signed : integral_constant<bool, T(-1) < T(0)> {}; 
    template<typename T>
    struct is_signed<T, false> : false_type {};
} 
template<typename T>
struct is_signed : detail::is_signed<T>::type {};

namespace detail{
    template<typename T, bool = is_arithmetic<T>::value>
    struct is_unsigned : integral_constant<bool, T(0) < T(-1) > {};
    template<typename T>
    struct is_unsigned<T, false> : false_type {};
}
template<typename T, typename enable_if<is_arithmetic<T>::value, bool>::type = true>
struct is_unsigned : detail::is_unsigned<T>::type {};

template<typename T>
struct is_bounded_array : false_type {};
template<typename T, std::size_t N>
struct is_bounded_array<T[N]> : true_type {};

template<typename T>
struct is_unbounded_array : false_type {};
template<typename T>
struct is_unbounded_array<T[]> : true_type {};

/*
*   SUPPORTED OPERATIONS
*/
template<typename T, typename U>
struct is_convertible{
private:
    typedef char No;
    typedef char (&Yes)[2];

    template<typename V, typename W>
    static Yes test(int(*)[sizeof(static_cast<V>(*((typename remove_reference<W>::type*)0)))]);
    template<typename, typename>
    static No test(...);

    public:
    static const bool value = sizeof(test<T,U>(0)) == sizeof(Yes);   
};

template<typename T>
struct is_default_constructible{
    private:
    typedef char No;
    typedef char (&Yes)[2];

    template<typename U>
    static Yes test(int(*)[sizeof(new U)]);
    template<typename>
    static No test(...);

    public:
    static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
};

namespace detail{
    template<typename T, typename Arg>
    struct is_constructible_from_one {
        private:
        typedef char No;
        typedef char (&Yes)[2];

        template<typename U, typename V>
        static Yes test(int(*)[sizeof(typename type_identity<U>::type(static_cast<V>(*((typename remove_reference<V>::type*)0))))]);
        template<typename, typename>
        static No test(...);
        
        public:
        static const bool value = sizeof(test<T,Arg>(0)) == sizeof(Yes);
    }; 

}
template<typename T>
struct is_copy_constructible : detail::is_constructible_from_one<T, typename add_lvalue_reference<T>::type> {};



template<typename T, typename Arg>
struct is_assignable{
    private:
    typedef char No;
    typedef char (&Yes)[2];

    template<typename U, typename V>
    static Yes test(int(*)[sizeof(detail::declval<U>() = detail::declval<V>())]);
    template<typename, typename>
    static No test(...);

    public:
    static const bool value = sizeof(test<T,Arg>(0)) == sizeof(Yes);
};

template<typename T>
struct is_copy_assignable : is_assignable<T, typename add_lvalue_reference<T>::type> {};

template<typename T>
struct is_destructible{
    private:
    typedef char No;
    typedef char (&Yes)[2];

    template<typename U>
    static Yes test(int(*)[sizeof(detail::declval<U>().~U(),detail::declval<U>())]);    //Comma otherwise we'd end up with sizeof(void) which is invalid
    template<typename>
    static No test(...);

    public:
    static const bool value = sizeof(test<T>(0)) == sizeof(Yes);   
};

namespace detail{
    using std::swap;    //This cannot be at class scope so we need a namespace
    template<typename T,typename U>
    struct is_swappable_with{
        private:    
        typedef char No;
        typedef char (&Yes)[2];

        template<typename V, typename W>
        static Yes test(int(*)[sizeof(swap(detail::declval<V>(),detail::declval<W>()),  detail::declval<U>())]);
        template<typename, typename>
        static No test(...);

        public:
        static const bool value = sizeof(test<T,U>(0)) == sizeof(Yes);   
    };

}

template<typename T, typename U>
struct is_swappable_with : detail::is_swappable_with<T,U> {};

template<typename T>
struct is_swappable : detail::is_swappable_with<T,T> {};






/*
*   PROPERTY QUERIES
*/
template<typename T>
struct rank : integral_constant<std::size_t, 0> {};
template<typename T>
struct rank<T[]> : integral_constant<std::size_t, rank<T>::value + 1> {};
template<typename T, std::size_t N>
struct rank<T[N]> : integral_constant<std::size_t, rank<T>::value + 1> {};

template<typename T, unsigned int N = 0>
struct extent : integral_constant<std::size_t, 0> {};
template<typename T>
struct extent<T[], 0> : integral_constant<std::size_t, 0> {};
template<typename T, unsigned int N>
struct extent<T[], N> : extent<T, N - 1> {};
template<typename T, std::size_t I>
struct extent<T[I], 0> : integral_constant<std::size_t, I> {};
template<typename T, std::size_t I, unsigned int N>
struct extent<T[I], N> : extent<T, N - 1> {};


/*
* TYPE RELATIONSHIPS
*/
template<typename T, typename U>
struct is_same : false_type{};
template<typename T>
struct is_same<T,T> : true_type{};


template<typename B, typename D>
struct is_base_of{
    private:
    typedef char No;
    typedef char (&Yes)[2];

    static Yes test(B*);
    static No test(...);
    public:
    static const bool value = sizeof(test(static_cast<D*>(0))) == sizeof(Yes);
};


/*
* TYPE MODIFIERS
*/
template<typename T>
struct remove_const{
    typedef T type;
};
template<typename T>
struct remove_const<const T>{
    typedef T type;
};

template<typename T>
struct remove_volatile{
    typedef T type;
};
template<typename T>
struct remove_volatile<volatile T>{
    typedef T type;
};

template<typename T>
struct remove_cv{
    typedef T type;
};
template<typename T>
struct remove_cv<const T>{
    typedef T type;
};
template<typename T>
struct remove_cv<volatile T>{
    typedef T type;
};
template<typename T>
struct remove_cv<const volatile T>{
    typedef T type;
};

template<typename T>
struct add_const{
    typedef const T type;
};

template<typename T>
struct add_volatile{
    typedef volatile T type;
};

template<typename T>
struct add_cv{
    typedef const volatile T type;
};

/*
* REFERENCES
*/
//NB: No rvalue references
template<typename T>
struct remove_reference{
    typedef T type;
};
template<typename T>
struct remove_reference<T&>{
    typedef T type;
};
//Not perfect, but it should work
template<typename T>
struct add_lvalue_reference{
    typedef T& type;
};
template<typename T>
struct add_lvalue_reference<T&>{
    typedef T& type;
};
template<>
struct add_lvalue_reference<void>{};

/*
* ARRAY TYPES
*/
template<typename T>
struct remove_extent{
    typedef T type;
};
template<typename T>
struct remove_extent<T[]>{
    typedef T type;
};
template<typename T, std::size_t N>
struct remove_extent<T[N]>{
    typedef T type;
};

template<typename T>
struct remove_all_extents{
    typedef T type;
};
template<typename T>
struct remove_all_extents<T[]> {
    typedef typename remove_all_extents<T>::type type;
}; 
template<typename T, std::size_t N>
struct remove_all_extents<T[N]> {
    typedef typename remove_all_extents<T>::type type;
};

/*
* POINTER TYPES
*/
template<typename T>
struct remove_pointer{
    typedef T type;
};
template<typename T>
struct remove_pointer<T*>{
    typedef T type;
};
template<typename T>
struct remove_pointer<T* const>{
    typedef T type;
};
template<typename T>
struct remove_pointer<T* volatile>{
    typedef T type;
};
template<typename T>
struct remove_pointer<T* const volatile>{
    typedef T type;
};

template<typename T>
struct add_pointer{
    typedef T* type;
};


/*
* MISC
*/
template<bool b, typename T>
struct enable_if{};

template<typename T>
struct enable_if<true,T>{
    typedef T type;
};

template<bool b, typename trueT, typename falseT>
struct conditional{
    typedef falseT type;
};

template<typename trueT, typename falseT>
struct conditional<true, trueT, falseT>{
    typedef trueT type;
};

template<typename T>
struct decay{
    private:
    typedef typename dp::remove_reference<T>::type U;
    public:
    typedef typename dp::conditional<
        dp::is_array<U>::value, 
        typename dp::add_pointer<typename dp::remove_extent<U>::type>::type,
        typename dp::conditional<
            dp::is_function<U>::value,
            typename dp::add_pointer<U>::type,
            typename dp::remove_cv<U>::type
        >::type
    >::type type;
};

template<typename T>
struct remove_cvref{
    typedef typename remove_cv<typename remove_reference<T>::type>::type type;
};

template<typename T>
struct type_identity{
    typedef T type;
};

/*
* SIGN MODIFIERS
* Down here because we need to use conditional as a member.
*/
namespace detail{
    //Signed non-bool, non-wchar_t types should all "return" the same type, unchanged
    template<typename T, bool = is_signed<T>::value && !is_same<T,bool>::value && !is_same<T,wchar_t>::value>
    struct make_signed{
        typedef T type;
    };
    //All other non-specialised types can't be used
    template<typename T>
    struct make_signed<T,false> {};
    //And here are the specialisations for specific types
    //Unfortunately we have to grind them out by hand as typedef signed T type isn't a thing.
    template<>
    struct make_signed<unsigned char, false>{
        typedef signed char type;
    };
    template<>
    struct make_signed<unsigned short, false>{
        typedef signed short type;
    };
    template<>
    struct make_signed<unsigned int, false>{
        typedef signed int type;
    };
    template<>
    struct make_signed<unsigned long, false>{
        typedef signed long type;
    };
    //Char types have a specific form of conversion we need
    //char avoid this by having signed and unsigned variants; wchar_t does not
    template<>
    struct make_signed<wchar_t, false>{
        typedef typename conditional<sizeof(int) >= sizeof(wchar_t), signed int, signed long>::type type;
    };
}

template<typename T>
struct make_signed : detail::make_signed<T> {};

namespace detail{
    template<typename T, bool = is_unsigned<T>::value && !is_same<T,bool>::value && !is_same<T,wchar_t>::value>
    struct make_unsigned{
        typedef T type;
    };
    template<typename T>
    struct make_unsigned<T,false> {};
    template<>
    struct make_unsigned<signed char, false>{
        typedef unsigned char type;
    };
    template<>
    struct make_unsigned<signed short, false>{
        typedef unsigned short type;
    };
    template<>
    struct make_unsigned<signed int, false>{
        typedef unsigned int type;
    };
    template<>
    struct make_unsigned<signed long, false>{
        typedef unsigned long type;
    };
    template<>
    struct make_unsigned<wchar_t, false>{
        typedef typename conditional<sizeof(unsigned int) >= sizeof(wchar_t), 
                                        unsigned int, unsigned long>::type type;
    };
}

template<typename T>
struct make_unsigned : detail::make_unsigned<T> {};

}

#endif