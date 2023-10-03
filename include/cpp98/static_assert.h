#ifndef DP_CPP98_STATIC_ASSERT
#define DP_CPP98_STATIC_ASSERT

#include "bits/static_assert_no_macro.h"

#if !defined STATIC_ASSERT && !defined DP_NO_ASSERT_MACRO
//#define STATIC_ASSERT(args) dp::static_assert_98<args>();
#define STATIC_ASSERT(...) dp::static_assert_98<__VA_ARGS__>();
#endif


#endif		//Header guard