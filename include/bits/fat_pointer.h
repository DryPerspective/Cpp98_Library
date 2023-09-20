#ifndef DP_CPP98_FAT_POINTER
#define DP_CPP98_FAT_POINTER

#include <cctype>

namespace dp {

	/*
	*  Not a true fat pointer, but a close enough approximation for our needs
	*  Meets size criteria and allows us to redefine later if a better way is found
	*/
	template<typename T>
	struct fat_pointer {
		T*			begin;
		std::size_t size;

		operator T* () { return begin; }
		operator const T* () const { return begin; }

		//Who doesn't love a lack of aggregate initialization and extended init lists
		fat_pointer(T* b, std::size_t s) : begin(b), size(s) {}
	};


}



#endif