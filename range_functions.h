#ifndef CPP98_RANGE_FUNCTIONS
#define CPP98_RANGE_FUNCTIONS

#include <cstddef>
#include <iterator>
#include <vector>

#include "type_traits.h"

namespace dp{
    template<typename T>
    typename T::iterator begin(T& in){
        return in.begin();
    }
    template<typename T>
    typename T::const_iterator begin(const T& in){
        return in.begin();
    }    
    template<typename T>
    typename T::const_iterator cbegin(const T& in){
        return in.begin();
    }
    template<typename T, std::size_t N>
    T* begin(T (&in)[N]){
        return in;
    }
    template<typename T, std::size_t N>
    const T* begin(const T (&in)[N]){
        return in;
    }
    template<typename T, std::size_t N>
    const T* cbegin(const T (&in)[N]){
        return in;
    }

    template<typename T>
    typename T::iterator end(T& in){
        return in.end();
    }
    template<typename T>
    typename T::const_iterator end(const T& in){
        return in.end();
    }
    template<typename T>
    typename T::const_iterator cend(const T& in){
        return in.end();
    }
    template<typename T, std::size_t N>
    T* end(T (&in)[N]){
        return in + N;
    }
    template<typename T, std::size_t N>
    const T* end(const T (&in)[N]){
        return in + N;
    }
    template<typename T, std::size_t N>
    const T* cend(const T (&in)[N]){
        return in + N;
    }

    template<typename T>
    typename T::reverse_iterator rbegin(T& in){
        return in.rbegin();
    }
    template<typename T>
    typename T::const_reverse_iterator rbegin(const T& in){
        return in.rbegin();
    }
    template<typename T>
    typename T::const_reverse_iterator crbegin(const T& in){
        return in.rbegin();
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<T*> rbegin(T (&in)[N]){
        return std::reverse_iterator<T*>(end(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> rbegin(const T (&in)[N]){
        return std::reverse_iterator<const T*>(end(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> crbegin(const T (&in)[N]){
        return std::reverse_iterator<const T*>(end(in));
    }

    template<typename T>
    typename T::reverse_iterator rend(T& in){
        return in.rend();
    }
    template<typename T>
    typename T::const_reverse_iterator rend(const T& in){
        return in.rend();
    }
    template<typename T>
    typename T::const_reverse_iterator crend(const T& in){
        return in.rend();
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<T*> rend(T (&in)[N]){
        return std::reverse_iterator<T*>(begin(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> rend(const T (&in)[N]){
        return std::reverse_iterator<const T*>(begin(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> crend(const T (&in)[N]){
        return std::reverse_iterator<const T*>(end(in));
    }

    template<typename T>
    std::size_t size(const T& in){
        return in.size();
    }
    template<typename T, std::size_t N>
    std::size_t size(const T (&in)[N]){
        return N;
    }

    template<typename T>
    std::ptrdiff_t ssize(const T& in){
        return in.size();
    }
    template<typename T, std::size_t N>
    std::ptrdiff_t ssize(const T (&in)[N]){
        return N;
    }

    template<typename T>
    bool empty(const T& in){
        return in.empty();
    }
    template<typename T, std::size_t N>
    bool empty(const T (&in)[N]){
        return N == 0;   //ISO C++ forbids a size zero array, but C doesn't.
                         //Feel free to edit this to return false if you're sure it won't be used that way
    }

    //Data
    /*  What I wouldn't do for deduced return types and variadic templates
    *   Apologies that this is some ugly code.
    *   Rationale: No standard container in C++98 has more than 4 template parameters
    *   (even then, most don't have a data() function)
    *   As such, this data() should cover all standard library containers (the spec) 
    *   and the lion's share of all possible custom containers (a bonus).
    *
    *   You also get a bonus feature - since std::data() is a C++17 feature, the preprocessor
    *   switch below will swap all these ugly overloads out for a simple variadic which achieves
    *   the same thing. In case you want to use this C++17 feature on C++11 or C++14
    */
#ifndef DP_CPP_11_DATA
    template<template<class> class temp, typename T>
    T* data(temp<T>& in){
        return in.data();
    }
    template<template<class> class temp, typename T>
    const T* data(const temp<T>& in){
        return in.data();
    }
    template<template<class,class> class temp, typename T, typename U>
    T* data(temp<T,U>& in){
        return in.data();
    }
    template<template<class,class> class temp, typename T,typename U>
    const T* data(const temp<T,U>& in){
        return in.data();
    }
    template<template<class,class, class> class temp, typename T, typename U, typename V>
    T* data(temp<T,U,V>& in){
        return in.data();
    }
    template<template<class,class,class> class temp, typename T,typename U, typename V>
    const T* data(const temp<T,U,V>& in){
        return in.data();
    }
    template<template<class,class,class,class> class temp, typename T, typename U, typename V, typename W>
    T* data(temp<T,U,V,W>& in){
        return in.data();
    }
    template<template<class,class,class,class> class temp, typename T,typename U, typename V, typename W>
    const T* data(const temp<T,U,V,W>& in){
        return in.data();
    }
    
    /*
    *  Vector is complicated.
    *  Prior to C++03, its storage was not required to be contiguous. Almost every implementation still stored it that way, but it was not a requirement.
    *  Additionally, the data() member function was added via DR and so not every C++98/03 compiler will have it. Some will, some won't. And indeed it was
    *  not uncommon back in the day to use a workaround to get there.
    *  I can't require that the data is contiguous on the program level. Good luck to you if you're on an exotic implementation.
    *  However, I can provide data through the (most likely safer) data() member if it exists, and use the workaround otherwise.
    */
    namespace detail {
        template<typename T>
        struct HasDataMember {
            private:
                typedef char No;
                typedef char(&Yes)[2];

                template<typename U>
                static Yes test(int(*)[sizeof(detail::declval<U>().data())]);
                template<typename, typename>
                static No test(...);

            public:
                static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
           
        };
    }
    template<typename T>
    typename enable_if<!detail::HasDataMember<std::vector<T> >::value, T*>::type data(std::vector<T>& in){
        return &in[0];
    }
    template<typename T>
    typename enable_if<!detail::HasDataMember<std::vector<T> >::value, const T*>::type data(const std::vector<T>& in){
        return &in[0];
    }

#else
    template<typename T>
    auto data(T& in) -> decltype(in.data()){
        return in.data();
    }
    template<typename T>
    auto data(const T& in) -> decltype(in.data()){
        return in.data();
    }
#endif

    //Overload for if you're using an std::array-style analogue
    template<template<class,std::size_t> class temp, typename T, std::size_t N>
    T* data(temp<T,N>& in){
        return in.data();
    }
    template<template<class,std::size_t> class temp, typename T, std::size_t N>
    const T* data(const temp<T,N>& in){
        return in.data();
    }

    //And after all this ugliness, we get to C-arrays
    template<typename T, std::size_t N>
    T* data(T (&in)[N]){
        return in;
    }
    template<typename T, std::size_t N>
    const T* data(const T (&in)[N]){
        return in;
    }




    /*
    *   These are not a part of a modern standard, and should be swapped out for auto as needed
    *   however, they make use of begin(), end(), etc more useful
    *   The idea being that instead of auto it = std::begin(range); you can do typename dp::iterator_type<T>::type it = dp::begin(range)
    *   Though hopefully you'd use a typedef to cut down on the verbosity.
    *   Don't forget to check const correctness manually. A const T which tries to use iterator_type<T> will probably create compile issues
    */
    namespace detail {
        template<typename T, bool = is_array<T>::value>
        struct iter_type {
            typedef typename T::iterator type;
        };
        template<typename T>
        struct iter_type<T, true> {
            typedef typename decay_array<T>::type type;
        };

        template<typename T, bool = is_array<T>::value>
        struct citer_type {
            typedef typename T::const_iterator type;
        };
        template<typename T>
        struct citer_type<T, true> {
            typedef typename decay_array<typename add_const<T>::type>::type type;
        };

        template<typename T, bool = is_array<T>::value>
        struct riter_type {
            typedef typename T::reverse_iterator type;
        };
        template<typename T>
        struct riter_type<T, true> {
            typedef typename std::reverse_iterator<typename decay_array<typename T>::type> type;
        };

        template<typename T, bool = is_array<T>::value>
        struct criter_type {
            typedef typename T::const_reverse_iterator type;
        };
        template<typename T>
        struct criter_type<T, true> {
            typedef typename std::reverse_iterator<typename decay_array<typename add_const<T>::type>::type> type;
        };

    } 

    template<typename T>
    struct iterator_type : detail::iter_type<T> {};
    template<typename T>
    struct const_iterator_type : detail::citer_type<T> {};
    template<typename T>
    struct reverse_iterator_type : detail::riter_type<T> {};
    template<typename T>
    struct const_reverse_iterator_type : detail::criter_type<T> {};

}

#endif