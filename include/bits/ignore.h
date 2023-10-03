#ifndef DP_CPP98_IGNORE
#define DP_CPP98_IGNORE

/*
*  An "ignore" class. A class which can be assigned to by anything but which doesn't do anything and cannot be converted to anything.
*  Good for placeholders, and compilers which are non-compliant in their eagerness to create ambiguous calls from type conversions *cough* Borland *cough*
*/
namespace dp {
	struct ignore {
		template<typename T>
        ignore(const T&){}

		template<typename T>
		void operator=(const T&) {}
	};
}

#endif