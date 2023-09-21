#ifndef DP_CPP98_ITERATOR
#define DP_CPP98_ITERATOR

#include <cstddef>
#include <iterator>
#include <vector>       //Vector special cases
#include <string>       //String special cases


#include "cpp98/type_traits.h"
#include "cpp98/utility.h"

/*
*   Much of the modern <iterator> header is C++20 concepts and changes to support C++20 ranges.
*   This library does not support either of those, so this header will be a lighter recreation than most.
*   I don't think it's unreasonable not to recreate move_iterators or objects which require requires requires to work
*/


namespace dp {


    /*
    *   ITERATOR FUNCTIONS
    */

    template<typename Iter1, typename Iter2>
    void iter_swap(Iter1 lhs, Iter2 rhs) {
        using std::swap;
        swap(*lhs, *rhs);
    }

    template<typename Iter>
    typename std::reverse_iterator<Iter> make_reverse_iterator(Iter in) {
        return std::reverse_iterator<Iter>(in);
    }


    template<typename InputIt>
    InputIt next(InputIt it, typename std::iterator_traits<InputIt>::difference_type n = 1) {
        std::advance(it, n);
        return it;
    }


    template<class BidirIt>
    BidirIt prev(BidirIt it, typename std::iterator_traits<BidirIt>::difference_type n = 1) {
        std::advance(it, -n);
        return it;
    }


    /*
    * RANGE FUNCTIONS
    */
    template<typename T>
    typename T::iterator begin(T& in) {
        return in.begin();
    }
    template<typename T>
    typename T::const_iterator begin(const T& in) {
        return in.begin();
    }
    template<typename T>
    typename T::const_iterator cbegin(const T& in) {
        return in.begin();
    }
    template<typename T, std::size_t N>
    T* begin(T(&in)[N]) {
        return in;
    }
    template<typename T, std::size_t N>
    const T* begin(const T(&in)[N]) {
        return in;
    }
    template<typename T, std::size_t N>
    const T* cbegin(const T(&in)[N]) {
        return in;
    }

    template<typename T>
    typename T::iterator end(T& in) {
        return in.end();
    }
    template<typename T>
    typename T::const_iterator end(const T& in) {
        return in.end();
    }
    template<typename T>
    typename T::const_iterator cend(const T& in) {
        return in.end();
    }
    template<typename T, std::size_t N>
    T* end(T(&in)[N]) {
        return in + N;
    }
    template<typename T, std::size_t N>
    const T* end(const T(&in)[N]) {
        return in + N;
    }
    template<typename T, std::size_t N>
    const T* cend(const T(&in)[N]) {
        return in + N;
    }

    template<typename T>
    typename T::reverse_iterator rbegin(T& in) {
        return in.rbegin();
    }
    template<typename T>
    typename T::const_reverse_iterator rbegin(const T& in) {
        return in.rbegin();
    }
    template<typename T>
    typename T::const_reverse_iterator crbegin(const T& in) {
        return in.rbegin();
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<T*> rbegin(T(&in)[N]) {
        return std::reverse_iterator<T*>(end(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> rbegin(const T(&in)[N]) {
        return std::reverse_iterator<const T*>(end(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> crbegin(const T(&in)[N]) {
        return std::reverse_iterator<const T*>(end(in));
    }

    template<typename T>
    typename T::reverse_iterator rend(T& in) {
        return in.rend();
    }
    template<typename T>
    typename T::const_reverse_iterator rend(const T& in) {
        return in.rend();
    }
    template<typename T>
    typename T::const_reverse_iterator crend(const T& in) {
        return in.rend();
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<T*> rend(T(&in)[N]) {
        return std::reverse_iterator<T*>(begin(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> rend(const T(&in)[N]) {
        return std::reverse_iterator<const T*>(begin(in));
    }
    template<typename T, std::size_t N>
    typename std::reverse_iterator<const T*> crend(const T(&in)[N]) {
        return std::reverse_iterator<const T*>(begin(in));
    }

    template<typename T>
    std::size_t size(const T& in) {
        return in.size();
    }
    template<typename T, std::size_t N>
    std::size_t size(const T(&)[N]) {
        return N;
    }

    template<typename T>
    std::ptrdiff_t ssize(const T& in) {
        return in.size();
    }
    template<typename T, std::size_t N>
    std::ptrdiff_t ssize(const T(&)[N]) {
        return N;
    }

    template<typename T>
    bool empty(const T& in) {
        return in.empty();
    }
    template<typename T, std::size_t N>
    bool empty(const T(&)[N]) {
        return N == 0;   //ISO C++ forbids a size zero array, but C doesn't.
        //Feel free to edit this to return false if you're sure it won't be used that way
    }

    //Data
    /*  What I wouldn't do for deduced return types and variadic templates
    *   Assuming a container/object follows the standard approach and provides standard typedefs, this will work.
    *
    *   Also secret bonus feature - since std::data is a C++17 feature, the following preprocessor switch will allow access to a C++11-friendly
    *   and more reliable variadic approach, which should hold for C++11 and C++14.
    */
#ifndef DP_CPP_11_DATA

    template<typename T>
    typename T::pointer data(T& in) {
        return in.data();
    }

    template<typename T>
    typename T::const_pointer data(const T& in) {
        return in.data();
    }

    /*
    *  Specialisation for strings
    *  Because const char* is the eternal special case and we can't have nice things.
    *  Always return const char, otherwise substitution fails. Because string::data() always returned a const char* until C++17
    *
    *  We start with a flat specialisation for the std::basic_string template
    *  Don't forget that prior to C++11, it's UB to dereference a pointer produced by std::string::data when the string is empty.
    */
    template<typename charT, typename charTraits, typename alloc>
    typename std::basic_string<charT, charTraits, alloc>::const_pointer data(std::basic_string<charT, charTraits, alloc>& in) {
        return in.data();
    }
    template<typename charT, typename charTraits, typename alloc>
    typename std::basic_string<charT, charTraits, alloc>::const_pointer data(const std::basic_string<charT, charTraits, alloc>& in) {
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
            template<typename>
            static No test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(Yes);

        };
    }
    template<typename T>
    typename dp::enable_if<!detail::HasDataMember<std::vector<T> >::value, T*>::type data(std::vector<T>& in) {
        return &in[0];
    }
    template<typename T>
    typename dp::enable_if<!detail::HasDataMember<std::vector<T> >::value, const T*>::type data(const std::vector<T>& in) {
        return &in[0];
    }

#else
    template<typename T>
    auto data(T& in) -> decltype(in.data()) {
        return in.data();
    }
    template<typename T>
    auto data(const T& in) -> decltype(in.data()) {
        return in.data();
    }
#endif

    //And after all this ugliness, we get to C-arrays
    template<typename T, std::size_t N>
    T* data(T(&in)[N]) {
        return in;
    }
    template<typename T, std::size_t N>
    const T* data(const T(&in)[N]) {
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
        template<typename T, bool = dp::is_array<T>::value>
        struct iter_type {
            typedef typename T::iterator type;
        };
        template<typename T>
        struct iter_type<T, true> {
            typedef typename dp::decay<T>::type type;
        };

        template<typename T, bool = dp::is_array<T>::value>
        struct citer_type {
            typedef typename T::const_iterator type;
        };
        template<typename T>
        struct citer_type<T, true> {
            typedef typename dp::decay<typename dp::add_const<T>::type>::type type;
        };

        template<typename T, bool = dp::is_array<T>::value>
        struct riter_type {
            typedef typename T::reverse_iterator type;
        };
        template<typename T>
        struct riter_type<T, true> {
            typedef typename std::reverse_iterator<typename dp::decay<T>::type> type;
        };

        template<typename T, bool = dp::is_array<T>::value>
        struct criter_type {
            typedef typename T::const_reverse_iterator type;
        };
        template<typename T>
        struct criter_type<T, true> {
            typedef typename std::reverse_iterator<typename dp::decay<typename dp::add_const<T>::type>::type> type;
        };

    }

    template<typename T>
    struct iterator_type : detail::iter_type<T> {};
    //Specialisation so that const T -> const_iterator
    //Note that for a function void f(const T& in), T will still deduce to a non-const type, and iterator_type<T> will give you a T::iterator
    //But if you feed a const variable into a function void f(T& in), this will give you a T::const_iterator and prevent const conversion errors.
    template<typename T>
    struct iterator_type<const T> : detail::citer_type<T> {};
    template<typename T>
    struct const_iterator_type : detail::citer_type<T> {};
    template<typename T>
    struct reverse_iterator_type : detail::riter_type<T> {};
    template<typename T>
    struct reverse_iterator_type<const T> : detail::criter_type<T> {};
    template<typename T>
    struct const_reverse_iterator_type : detail::criter_type<T> {};

}

#endif