#ifndef DP_CPP98_FOR_EACH
#define DP_CPP98_FOR_EACH

#include "cpp98/iterator.h"

//WIP
#ifndef DP_CPP11_OR_LATER
#define FOR_EACH(container_type, elem, container_name) for(typename dp::iterator_type<container_type>::type elem = dp::begin(container_name), End = dp::end(container_name); elem != End; ++elem) 
#else
#define FOR_EACH(container_type, elem, container_name) for(const auto& elem : container_name)
#endif

#endif