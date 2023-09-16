#ifndef DP_CPP98_STATIC_ASSERT
#define DP_CPP98_STATIC_ASSERT


namespace dp{
    template<bool Condition>
    struct static_assert_98{};

    //If you get an error pointing here saying that you're
    //trying to use an incomplete type, your assertion failed.
    //Address that, not this. This doesn't need fixing.
    template<>
    struct static_assert_98<false>;
}

#if !defined STATIC_ASSERT && !defined DP_NO_ASSERT_MACRO
#define STATIC_ASSERT(args) dp::static_assert_98<args>();
#endif


#endif		//Header guard