#ifndef DP_CPP98_STATIC_ASSERT
#define DP_CPP98_STATIC_ASSERT

#include "bits/static_assert_no_macro.h"

#if !defined STATIC_ASSERT && !defined DP_NO_ASSERT_MACRO
#define STATIC_ASSERT(args) dp::static_assert_98<args>();
#endif


#endif		//Header guard