#ifndef DP_CPP98_VERSION_DEFS
#define DP_CPP98_VERSION_DEFS

/*
*  This is a C++98 library. I don't pretend it's anything else, and I don't recommend its use in modern code.
*  But, there may exist a fringe case where during the update process a user may want it to be active on a later language standard.
*  While I will try to avoid adding or interacting with a feature which was deprecated or removed in a future standard, I will use some
*  if I judge that in the purview of a C++98 library, it would be more detrimental to omit it in C++98 code than it would be to 
*  use it even if that means less future compatibility.
*  I wrap such code in these so that some unrelated part of your code doesn't torpedo the update process.
*  e.g. the smart pointers in this C++98 lib support auto_ptr usage, but if you've tidied yourself up on that already, using an appropriate
*  define here won't mean the fact that the interface happens to contain auto_ptr from being a thorn in your side in the C++17 update process.
*/


#if defined(DP_CPP11)
#define DP_CPP11_OR_HIGHER
#elif defined(DP_CPP14)
#define DP_CPP11_OR_HIGHER
#define DP_CPP14_OR_HIGHER
#elif defined(DP_CPP17)
#define DP_CPP11_OR_HIGHER
#define DP_CPP14_OR_HIGHER
#define DP_CPP17_OR_HIGHER
#elif defined(DP_CPP20)
#define DP_CPP11_OR_HIGHER
#define DP_CPP14_OR_HIGHER
#define DP_CPP17_OR_HIGHER
#define DP_CPP20_OR_HIGHER
#elif defined(CP_CPP23)
#define DP_CPP11_OR_HIGHER
#define DP_CPP14_OR_HIGHER
#define DP_CPP17_OR_HIGHER
#define DP_CPP20_OR_HIGHER
#define DP_CPP23_OR_HIGHER
#endif

#if defined(__BORLANDC__) && __BORLANDC__ < 0x0740
/*
*  Borland's *wonderful* compiler cannot do a great many things. These things include but are not limited to:
*  Using function overloads when resolving templates (goodbye SFINAE)
*  Disambiguating array templates T[] vs T[N] (bounded and unbounded arrays are the same, apparently)
*  Recognising template functions without complaining that it doesn't know they are a function (make_shared generated an error "templates must be classes or functions" a fair few times)
*  
*  As such it has become increasingly impractical put a full if !defined(__BORLANDC__) || __BORLANDC__ >= 0x740 all over the code. Instead we do this.
*  I expect this will become a common fixture in the library
*/
#define DP_BORLAND
#endif


#if defined(DP_BORLAND) && !defined(DP_BORLAND_USE_STD_EXCEPTIONS)
#define DP_BORLAND_EXCEPIONS
#endif




#endif