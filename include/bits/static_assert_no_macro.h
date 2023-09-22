#ifndef DP_CPP98_BITS_STATIC_ASSERT_NO_MACRO
#define DP_CPP98_BITS_STATIC_ASSERT_NO_MACRO

/*
*  It's all well and good writing a static assertion tool with a helper macro, but we don't want a bunch of other headers which use it
*  polluting the "macro namespace" with something which the user may not want defined, and needing to order a #define DP_NO_ASSERT_MACRO before
*  every include of one of any number of unrelated headers on the user end is not something they will appreciate.
*  As such, all internal static assertions are done here, with no macro in sight. And the static assertion header
*  Just includes this and adds the macro. No awkward macro logic needed.
*/



namespace dp {
    template<bool Condition>
    struct static_assert_98 {};

    //If you get an error pointing here saying that you're
    //trying to use an incomplete type, your assertion failed.
    //Address that, not this. This doesn't need fixing.
    template<>
    struct static_assert_98<false>;
}



#endif