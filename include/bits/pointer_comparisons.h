#ifndef DP_CPP98_POINTER_COMPARISONS
#define DP_CPP98_POINTER_COMPARISONS

#include "cpp98/type_traits.h"
#include "cpp98/null_ptr.h"

#include "bits/type_traits_ns.h"

/*
*	This library supports several pointer types, all of which should be comparable to raw pointers and to null pointers.
*	But that's ~24 operator overloads per pointer, and who has time for that?
*   Far better to overengineer a template solution to enable pointer-like comparison operators for all "pointer-like" classes.
* 
*	Unfortunately it is generally not possible for a class which supports conversion to bool to be comparable to NULL.
*	This is because in C++98 conversion operations are always implicit (explicit is a C++11 feature) and NULL is an integral type.
*	A function where we compare the pointer class exactly with a pointer type will require as many conversions as converting the 
*   pointer class to bool and using the builtin so will be ambiguous. Comparing the pointer to an integral type directly is not permitted.
*   So, we can support conversion with pointer types and dp::null_ptr, but not with C++98 NULL.
*/

namespace dp {

	//Not all smart pointers are meant to be compared to raw pointers and not everything can be compared to a null pointer
	//That's the spec
	//So we separate them. Raw pointers first.


	//This class, specialised for a given pointer type, will allow the pointer comparison operations in this header
	template<template<class> class>
	struct enable_raw_pointer_comparisons_one_arg : dp::false_type {};

	template<template<class, class> class>
	struct enable_raw_pointer_comparisons_two_args : dp::false_type {};


	//And we need to be able to reflect on whether it has been specialised directly
	//Which is to say - given a type we need to figure out if it is a specialisation of a template
	//for which enable_raw_pointer_comparisons has been set to true
	template<typename>
	struct raw_pointer_comps_enabled : dp::false_type {};

	template<template<class> class Temp, typename Arg>
	struct raw_pointer_comps_enabled<Temp<Arg> > {
		static const bool value = enable_raw_pointer_comparisons_one_arg<Temp>::value;
	};

	template<template<class, class> class Temp, typename Arg1, typename Arg2>
	struct raw_pointer_comps_enabled<Temp<Arg1, Arg2> > {
		static const bool value = enable_raw_pointer_comparisons_two_args<Temp>::value;
	};


	//We choose not to define operators in terms of each other to prevent too many templates needing to be instantiated
	//Don't forget - in addition to the operators themselves there are ~3 templates to be generated here for the SFINAE
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator==(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() == rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator==(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() == lhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator!=(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() != rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator!=(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() != lhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator<(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() < rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator<(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() < lhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator<=(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() <= rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator<=(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() <= lhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator>(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() > rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator>(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() > lhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator>=(const T& lhs, typename dp::param_types<T>::first_param_type* rhs) {
		return lhs.get() >= rhs;
	}
	template<typename T>
	inline typename dp::enable_if<raw_pointer_comps_enabled<T>::value, bool>::type operator>=(typename dp::param_types<T>::first_param_type* lhs, const T& rhs) {
		return rhs.get() >= lhs;
	}



	//And null pointer comparison


	template<template <class> class>
	struct enable_null_ptr_comparison_one_arg : dp::false_type {};

	template<template <class, class> class>
	struct enable_null_ptr_comparison_two_args : dp::false_type {};

	template<typename>
	struct null_ptr_comps_enabled : dp::false_type {};

	template<template<class> class Temp, typename Arg>
	struct null_ptr_comps_enabled<Temp<Arg> > {
		static const bool value = enable_null_ptr_comparison_one_arg<Temp>::value;
	};

	template<template<class, class> class Temp, typename Arg1, typename Arg2>
	struct null_ptr_comps_enabled<Temp<Arg1, Arg2> > {
		static const bool value = enable_null_ptr_comparison_two_args<Temp>::value;
	};

	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator==(const T& ptr, dp::null_ptr_t) {
		return !ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator==(dp::null_ptr_t, const T& ptr) {
		return !ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator!=(const T& ptr, dp::null_ptr_t) {
		return ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator!=(dp::null_ptr_t, const T& ptr) {
		return ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator<(const T& ptr, dp::null_ptr_t) {
		return ptr.get() < dp::null_ptr;
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator<(dp::null_ptr_t, const T& ptr) {
		return dp::null_ptr < ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator<=(const T& ptr, dp::null_ptr_t) {
		return ptr.get() <= dp::null_ptr;
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator<=(dp::null_ptr_t, const T& ptr) {
		return dp::null_ptr <= ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator>(const T& ptr, dp::null_ptr_t) {
		return ptr.get() > dp::null_ptr;
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator>(dp::null_ptr_t, const T& ptr) {
		return dp::null_ptr > ptr.get();
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator>=(const T& ptr, dp::null_ptr_t) {
		return ptr.get() >= dp::null_ptr;
	}
	template<typename T>
	inline typename dp::enable_if<null_ptr_comps_enabled<T>::value, bool>::type operator>=(dp::null_ptr_t, const T& ptr) {
		return dp::null_ptr >= ptr.get();
	}
	


}




#endif