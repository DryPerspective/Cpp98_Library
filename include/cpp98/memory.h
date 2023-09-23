#ifndef DP_CPP98_MEMORY
#define DP_CPP98_MEMORY

/*
*	As I offer smart pointeers as entirely distinct heeaders anyway, we increase modularity by collapsing the memory header down to these three includes.
*	In case you're having trouble figuring it out, bits/misc_memory_functions are where you'll find all the parts of <memory> which are not shared ptrs or scoped ptrs
*/

#include "bits/misc_memory_functions.h"
#include "cpp98/scoped_ptr.h"
#include "cpp98/shared_ptr.h"

#endif