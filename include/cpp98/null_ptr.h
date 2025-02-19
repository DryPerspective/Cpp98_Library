#ifndef DP_CPP98_NULL_PTR
#define DP_CPP98_NULL_PTR

#include "bits/version_defs.h"

/*
*  As with static_assert, we can't replicate a core language feature fully, nor can we use the same spelling; as this creates a warning on many compilers in C++98 mode
* 
*  Full documentation at: https://github.com/DryPerspective/Cpp98_Library/wiki/Null_ptr
*/
namespace dp {
	struct null_ptr_t {

		template<typename T>
		operator T* () const volatile {
			return NULL;
		}
	};

	static const null_ptr_t null_ptr
	#ifdef DP_BORLAND
	= {}
	#endif
	;
}

#endif