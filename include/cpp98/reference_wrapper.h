#ifndef DP_CPP98_REFERENCE_WRAPPER
#define DP_CPP98_REFERENCE_WRAPPER

#include "cpp98/type_traits.h"
#include "bits/misc_memory_functions.h"

namespace dp {



	template<typename T>
	class reference_wrapper {
	public:		
		
		typedef T type;

		template<typename Val>
		reference_wrapper(Val& in, typename dp::enable_if<!dp::is_same<reference_wrapper, typename dp::remove_cvref<Val>::type>::value, bool>::type = true) : m_data(dp::addressof(in)) {}

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

#endif