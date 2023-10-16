#ifndef DP_CPP98_NEW
#define DP_CPP98_NEW

#include <new>

namespace dp {

	//Obviously not concurrency aware...
	std::new_handler get_new_handler() {
		std::new_handler temp = std::set_new_handler(NULL);
		std::set_new_handler(temp);
		return temp;
	}
}

#endif