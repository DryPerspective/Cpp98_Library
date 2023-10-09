#ifndef DP_CPP98_NULL_PTR
#define DP_CPP98_NULL_PTR

namespace dp {
	struct null_ptr_t {

		template<typename T>
		operator T* () const {
			return NULL;
		}
	};

	static const null_ptr_t null_ptr;
}

#endif