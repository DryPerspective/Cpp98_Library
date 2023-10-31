#ifndef DP_CPP98_MISC_MEMORY
#define DP_CPP98_MISC_MEMORY


/*
*	I offer the smart pointer classes usually found in <memory> as separate headers, meaning that the overall header of memory.h will include these.
*	However, a few miscellaneous functions in the modern <memory> header such as addressof and construct_at can see common use throughout the library.
*	Given those two things, the decision was made to relegate the contents of <memory> which do not pertain to smart pointers here, and include the three
*   in the complete memory.h header. 
*   This allows memory.h to have an interface consistent with the standard while not dragging smart pointers and every last bit of it into every other header
*   which just wants to use a single memory function.
*/


#include <cstddef>
#include <iterator>

#include "cpp98/type_traits.h"
#include "bits/type_traits_ns.h"
#include "bits/static_assert_no_macro.h"
#include "bits/version_defs.h"

namespace dp {

    //Forward decs
    template<typename T>
    T* addressof(T&);

    template<typename ForwardIt>
    void destroy(ForwardIt begin, ForwardIt end);


    /*
    *  POINTER TRAITS
    *  Conditional typedefs are awkward. We can't just dp::conditional it because both possible types need to be well-formed
    *  So we take the longer way around.
    */

#ifndef DP_BORLAND
	namespace detail {
        template<typename T>
        struct HasElemTypeMember {
        private:
            typedef char No;
            typedef char(&Yes)[2];

            template<typename U>
            static Yes test(int(*)[sizeof(typename U::element_type)]);
            template<typename>
            static No test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
        };

        template<typename T>
        struct HasDiffTypeMember {
        private:
            typedef char No;
            typedef char(&Yes)[2];

            template<typename U>
            static Yes test(int(*)[sizeof(typename U::difference_type)]);
            template<typename>
            static No test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
        };
        //Base - element type
        template<typename T, bool = dp::detail::HasElemTypeMember<T>::value>
        struct pointer_traits_impl_elem {
            typedef typename T::element_type element_type;
        };
        template<typename T>
        struct pointer_traits_impl_elem<T, false> {
            typedef typename dp::param_types<T>::first_param_type element_type;
        };

        //Next level up - difference type
        template<typename T, bool = dp::detail::HasDiffTypeMember<T>::value>
        struct pointer_traits_impl_diff : dp::detail::pointer_traits_impl_elem<T> {
            typedef typename T::difference_type difference_type;
        };
        template<typename T>
        struct pointer_traits_impl_diff<T, false> : dp::detail::pointer_traits_impl_elem<T> {
            typedef std::ptrdiff_t difference_type;
        };
    }
    //And our final trait, which will selectively inherit the correct typedefs depending on T
    template<typename T>
    struct pointer_traits : dp::detail::pointer_traits_impl_diff<T> {
        typedef T pointer;

        //Fortunately the standard specifies that we can simply error out if T::pointer_to does not exist.
        static pointer pointer_to(typename dp::detail::pointer_traits_impl_elem<T>::element_type& r) {
            return T::pointer_to(r);
        }

        //Rebind requires variadic templates, which we don't have
    };

    template<typename T>
    struct pointer_traits<T*> {
        typedef T* pointer;
        typedef T               element_type;
        typedef std::ptrdiff_t  difference_type;

        static pointer pointer_to(element_type& r) {
            return dp::addressof(r);
        }

        template<typename U>
        struct rebind {
            typedef U* type;
        };
    };

    /*
    *  Allocator traits requires a general-case rebind in pointer_traits, which requires variadic templates.
    *  Alas, I cannot implement it.
    */

    //Onwards and upwards - Uses allocator
    namespace detail {
        template<typename T>
        struct HasAllocTypeMember {
        private:
            typedef char No;
            typedef char(&Yes)[2];

            template<typename U>
            static Yes test(int(*)[sizeof(typename U::allocator_type)]);
            template<typename>
            static No test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
        };


        template<typename T, typename Alloc, bool = dp::detail::HasAllocTypeMember<T>::value>
        struct uses_allocator_impl {
            static const bool value = dp::is_convertible<Alloc, typename T::allocator_type>::value;
        };
        template<typename T, typename Alloc>
        struct uses_allocator_impl<T, Alloc, false> {
            static const bool value = false;
        };

    }

    template<typename T, typename Alloc>
    struct uses_allocator : dp::detail::uses_allocator_impl<T, Alloc> {};

    //To-address
    namespace detail {
        template<typename T>
        struct HasPtrToAddress {
        private:
            typedef char No;
            typedef char(&Yes)[2];

            template<typename U>
            static Yes test(int(*)[sizeof(dp::pointer_traits<T>::pointer_to(dp::detail::declval<T>()))]);
            template<typename>
            static No test(...);

        public:
            static const bool value = sizeof(test<T>(0)) == sizeof(Yes);
        };
    }

    template<typename T>
    T* to_address(T* p) {
        dp::static_assert_98<!dp::is_function<T>::value>();
        return p;
    }

	template<typename T>
    typename dp::enable_if<dp::detail::HasPtrToAddress<T>::value, T*>::type to_address(const T& inPtr) {
        return dp::pointer_traits<T>::pointer_to(inPtr);
    }
    template<typename T>
    typename dp::enable_if<!dp::detail::HasPtrToAddress<T>::value, T*>::type to_address(const T& inPtr) {
        return dp::to_address(inPtr.operator->());
    }
#endif
    template<typename T>
    T* addressof(T& in) {
        return reinterpret_cast<T*>(
            &const_cast<char&>(
                reinterpret_cast<const volatile char&>(in)));
    }

    template<class InputIt, class Size, class NoThrowForwardIt>
    NoThrowForwardIt uninitialized_copy_n(InputIt begin, Size count, NoThrowForwardIt d_begin)
    {
        typedef typename std::iterator_traits<NoThrowForwardIt>::value_type T;
        NoThrowForwardIt current = d_begin;
        try
        {
            for (; count > 0; ++begin, (void) ++current, --count)
                ::new (static_cast<void*>(dp::addressof(*current))) T(*begin);
        }
        catch (...)
        {
            for (; d_begin != current; ++d_begin)
                d_begin->~T();
            throw;
        }
        return current;
    }

    template<class ForwardIt>
    void uninitialized_default_construct(ForwardIt begin, ForwardIt end)
    {
        typedef typename std::iterator_traits<ForwardIt>::value_type Value;
        ForwardIt current = begin;
        try
        {
            for (; current != end; ++current)
            {
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    dp::addressof(*current)))) Value;
            }
        }
        catch (...)
        {
            dp::destroy(begin, current);
            throw;
        }
    }

    template<class ForwardIt, class Size>
    ForwardIt uninitialized_default_construct_n(ForwardIt begin, Size n)
    {
        typedef typename std::iterator_traits<ForwardIt>::value_type T;
        ForwardIt current = begin;

        try
        {
            for (; n > 0; (void) ++current, --n)
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    dp::addressof(*current)))) T;
            return current;
        }
        catch (...)
        {
            dp::destroy(begin, current);
            throw;
        }
    }


    template<class ForwardIt>
    void uninitialized_value_construct(ForwardIt begin, ForwardIt end)
    {
        typedef typename std::iterator_traits<ForwardIt>::value_type Value;
        ForwardIt current = begin;
        try
        {
            for (; current != end; ++current)
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    dp::addressof(*current)))) Value();
        }
        catch (...)
        {
            dp::destroy(begin, current);
            throw;
        }
    }

    template<class ForwardIt, class Size>
    ForwardIt uninitialized_value_construct_n(ForwardIt begin, Size n)
    {
        typedef typename std::iterator_traits<ForwardIt>::value_type T;
        ForwardIt current = begin;
        try
        {
            for (; n > 0; (void) ++current, --n)
                ::new (const_cast<void*>(static_cast<const volatile void*>(
                    dp::addressof(*current)))) T();
            return current;
        }
        catch (...)
        {
            dp::destroy(begin, current);
            throw;
        }
    }

    template<typename T>
    typename dp::enable_if<is_bounded_array<T>::value, void>::type destroy_at(T* in) {
        for (std::size_t i = 0; i < dp::extent<T>::value; ++i) {
            destroy_at(dp::addressof(in[i]));
        }
    }
    template<typename T>
    typename dp::enable_if<!is_array<T>::value, void>::type destroy_at(T* in) {
        in->~T();
    }

    template<typename ForwardIt>
    void destroy(ForwardIt begin, ForwardIt end) {
        for (; begin != end; ++begin) dp::destroy_at(dp::addressof(*begin));
    }

    template<typename ForwardIt, typename Size>
    ForwardIt destroy_n(ForwardIt begin, Size n) {
        for (; n > 0; (void) ++begin, --n)  dp::destroy_at(dp::addressof(*begin));
        return begin;
    }






}





#endif