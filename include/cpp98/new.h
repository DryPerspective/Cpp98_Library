#ifndef DP_CPP98_NEW
#define DP_CPP98_NEW

#include <new>

namespace dp {
	namespace detail {
		//Exists just so we have something to set and reset.
		void dud_new_handler() {}
	}

	//Obviously not concurrency aware...
	std::new_handler get_new_handler() {
		std::new_handler temp = std::set_new_handler(dp::detail::dud_new_handler);
		std::set_new_handler(temp);
		return temp;
	}
}

#endif