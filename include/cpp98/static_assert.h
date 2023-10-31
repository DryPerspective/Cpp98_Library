#ifndef DP_CPP98_STATIC_ASSERT
#define DP_CPP98_STATIC_ASSERT

#include "bits/static_assert_no_macro.h"

#if !defined STATIC_ASSERT && !defined DP_NO_ASSERT_MACRO
//Sizeof is slightly more likely to be optimized out
#define STATIC_ASSERT(condition) sizeof(dp::static_assert_98<(condition)>);
#endif


#endif		//Header guard