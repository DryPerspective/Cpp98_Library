#ifndef DP_CPP98_NEW
#define DP_CPP98_NEW

#include <new>

// Full documentation at: https://github.com/DryPerspective/Cpp98_Library/wiki/New

namespace dp {

	//Obviously not concurrency aware...
	std::new_handler get_new_handler() {
		std::new_handler temp = std::set_new_handler(NULL);
		std::set_new_handler(temp);
		return temp;
	}
}

#endif