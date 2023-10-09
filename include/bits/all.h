#ifndef DP_CPP98_ALL
#define DP_CPP98_ALL

/*
*	DEBUG HEADER FOR TESTING
*	Yes, this includes the entire library. It's the easiest way to test that on first pass, things will behave
*	This should not be included in your own code.
*	If you include this for no reason except laziness, I reserve the right to judge your programming skills
*/

#include "bits/version_defs.h"

#include "bits/fat_pointer.h"
#include "bits/ignore.h"
#include "bits/misc_memory_functions.h"
#include "bits/smart_ptr_bases.h"
#include "bits/static_assert_no_macro.h"
#include "bits/type_traits_ns.h"
#include "bits/unbound_storage.h"

#include "cpp98/algorithm.h"
#include "cpp98/any.h"
#include "cpp98/array.h"
#include "cpp98/bit.h"
//Because byte contains a raw static assertion in the header
#ifndef DP_NO_INCLUDE_BYTE
#include "cpp98/byte.h"
#endif
#include "cpp98/cow_ptr.h"
#include "cpp98/expected.h"
#include "cpp98/iterator.h"
#include "cpp98/memory.h"
#include "cpp98/new.h"
#include "cpp98/null_ptr.h"
#include "cpp98/numeric.h"
#include "cpp98/optional.h"
#include "cpp98/ratio.h"
#include "cpp98/reference_wrapper.h"
#include "cpp98/scoped_ptr.h"
#include "cpp98/shared_ptr.h"
#include "cpp98/span.h"
#include "cpp98/static_assert.h"
#include "cpp98/string.h"
#include "cpp98/string_view.h"
#include "cpp98/type_traits.h"
#include "cpp98/typeindex.h"
#include "cpp98/utility.h"

#ifdef __BORLANDC__
#include "borland/borland_strings.h"
#include "borland/borland_deleters.h"
#include "borland/borland_raii.h"
#endif

#endif