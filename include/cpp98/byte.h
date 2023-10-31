#ifndef DP_CPP98_BYTE
#define DP_CPP98_BYTE

#include "bits/version_defs.h"

#include "bits/static_assert_no_macro.h"

#include "cpp98/type_traits.h"

/*
*	Implementing a byte is tricky.
*	There are no scoped enums we can use, as the standard does, and we can't use an unscoped enum as that's no better than a typedef for unsigned char.
*	If we're asserting that a byte is exactly one byte in size, we can't inherit from a static_assert class because at the point we're asserting the size of an
*	incomplete type; we can't composite because that would add size to the overall struct (and on one compiler this was tested on, empty classes were padded
*   up to a size of 8 bytes (no clue why), so that just leaves us with the option of a free floating assert. Which is clunky but will work.
*/

namespace dp {

//Borland compiler takes a creative interpretation of the standard for friend functions
//The simplest solution is to just avoid that mess. So yes, the Borland compiler has much weaker encapsulation
#ifndef DP_BORLAND

	class byte {
		unsigned char val;

		//We want to encapsulate val properly, so we unfortunately have to friend up every function which should be able to manipulate it directly.
		template<typename IntType>
		friend typename dp::enable_if<dp::is_integral<IntType>::value, IntType>::type to_integer(const byte&);

		template<typename IntType>
		friend typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte&>::type operator<<=(dp::byte& b, IntType in);

		template<typename IntType>
		friend typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte&>::type operator<<=(dp::byte& b, IntType in);

		friend inline dp::byte& operator|=(dp::byte& lhs, dp::byte rhs);
		friend inline dp::byte& operator&=(dp::byte& lhs, dp::byte rhs);
		friend inline dp::byte& operator^=(dp::byte& lhs, dp::byte rhs);

#else
	struct byte{

		unsigned char val;
#endif
	public:
		explicit byte() : val(0) {}
		explicit byte(unsigned char in) : val(in) {}	//To match C++17 relaxed enum initialization rules
	};



	//I don't like this approach either, but I don't have a lot of options
	//And to be honest, odds are this will never be a problem as almost all systems will have byte be one byte in size.
	static const dp::static_assert_98<sizeof(dp::byte) == 1> dp_assert_byte_size
	//Borland says that this must be initialised, C++ says it mustn't. Go figure.
#ifdef DP_BORLAND
	= {}
#endif
	;


	template<typename IntType>
	typename dp::enable_if<dp::is_integral<IntType>::value, IntType>::type to_integer(const dp::byte& in) {
		return in.val;
	}

	template<typename IntType>
	typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte&>::type operator<<=(dp::byte& b, IntType in) {
		b.val <<= in;
		return b;
	}

	template<typename IntType>
	typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte&>::type operator>>=(dp::byte& b, IntType in) {
		b.val >>= in;
		return b;
	}

	template<typename IntType>
	typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte>::type operator<<(dp::byte b, IntType in) {
		return b <<= in;
	}

	template<typename IntType>
	typename dp::enable_if<dp::is_integral<IntType>::value, dp::byte>::type operator>>(dp::byte b, IntType in) {
		return b >>= in;
	}

	inline dp::byte& operator|=(dp::byte& lhs, dp::byte rhs) {
		lhs.val |= rhs.val;
		return lhs;
	}
	inline dp::byte operator|(dp::byte lhs, dp::byte rhs) {
		return lhs |= rhs;
	}

	inline dp::byte& operator&=(dp::byte& lhs, dp::byte rhs) {
		lhs.val &= rhs.val;
		return lhs;
	}
	inline dp::byte operator&(dp::byte lhs, dp::byte rhs) {
		return lhs &= rhs;
	}

	inline dp::byte& operator^=(dp::byte& lhs, dp::byte rhs) {
		lhs.val ^= rhs.val;
		return lhs;
	}
	inline dp::byte operator^(dp::byte lhs, dp::byte rhs) {
		return lhs ^= rhs;
	}

}



#endif