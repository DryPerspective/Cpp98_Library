#ifndef DP_CPP98_REFERENCE_WRAPPER
#define DP_CPP98_REFERENCE_WRAPPER

#include "cpp98/type_traits.h"
#include "bits/misc_memory_functions.h"
#include "bits/version_defs.h"

#ifdef DP_BORLAND
#include "bits/ignore.h"
#define DP_ENABLE_TYPE dp::ignore_t
#else
#define DP_ENABLE_TYPE bool
#endif

//Full documentation at: https://github.com/DryPerspective/Cpp98_Library/wiki/Reference-Wrapper

namespace dp {


	namespace detail{
        //Just don't ask me why. But Borland seems to require this.
		template<typename Val, typename Wrapper>
		struct valid_ref_wrap_type{
			static const bool value = !dp::is_same<Wrapper, typename dp::remove_cvref<Val>::type>::value;
        };
    }


	template<typename T>
	class reference_wrapper {
	public:		
		
		typedef T type;

		template<typename Val>
		reference_wrapper(Val& in, typename dp::enable_if<detail::valid_ref_wrap_type<Val, reference_wrapper<T> >::value, DP_ENABLE_TYPE>::type = true) : m_data(dp::addressof(in)) {}

		reference_wrapper(const reference_wrapper& rhs) : m_data(rhs.m_data) {}
		reference_wrapper& operator=(const reference_wrapper& rhs) {
			m_data = rhs.m_data;
			return *this;
		}

		T& get() const {
			return *m_data;
		}

		operator T& () const {
			return get();
		}

	private:
		T* m_data;
	};


	template<typename T>
	dp::reference_wrapper<T> ref(T& t) {
		return dp::reference_wrapper<T>(t);
	}
	template<typename T>
	dp::reference_wrapper<T> ref(dp::reference_wrapper<T> t) {
		return t;
	}

	template<typename T>
	dp::reference_wrapper<const T> cref(const T& t) {
		return dp::reference_wrapper<const T>(t);
	}
	template<typename T>
	dp::reference_wrapper<const T> cref(dp::reference_wrapper<const T> t) {
		return t;
	}

}

#undef DP_ENABLE_TYPE

#endif