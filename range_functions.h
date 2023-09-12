#include <cstddef>
#include <iterator>
#include <vector>

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
    template<template<class,class, class,class> class temp, typename T, typename U, typename V, typename W>
    T* data(temp<T,U,V,W>& in){
        return in.data();
    }
    template<template<class,class,class,class> class temp, typename T,typename U, typename V, typename W>
    const T* data(const temp<T,U,V,W>& in){
        return in.data();
    }
    //Vector's data function was added retroactively via DR; and odds are if you're still
    //on a compiler that's still in C++98 then you probably also won't have been keeping
    //up with all the latest DRs either. So, we use this old trick.
    //Here's hoping you're either at least on C++03 or aren't using an exotic implementation
    //which doesn't guarantee contiguous memory layout.
    template<typename T>
    T* data(std::vector<T>& in){
        return &in[0];
    }
    template<typename T>
    const T* data(const std::vector<T>& in){
        return &in[0];
    }

#else
    template<template<class...> class temp, typename T>
    auto data(temp<T>& in) -> decltype(in.data()){
        return in.data();
    }
    template<template<class...> class temp, typename T>
    auto data(const temp<T>& in) -> decltype(in.data()){
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
}