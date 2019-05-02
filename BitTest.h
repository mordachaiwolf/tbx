//////////////////////////////////////////////////////////////
// Bit Manipulation Helper Functions
//
//  Note: we do the static_cast of other arguments to make it
//        easy to use with untyped or poorly typed values
//        such as anything in the windows.h headers
//////////////////////////////////////////////////////////////

#pragma once

// WinNT.h #define's a BitTest - ?! WTH ?!
#undef BitTest

namespace tbx {

	// returns true if the bits are identical for total bits in left side type
	// a == b
	template <typename T, typename U>
	inline bool BitCompare(T lhs, U rhs)
	{
		// note: we use the common unsigned type in order to ensure no sign extension is performed anywhere!
		using C = typename std::common_type<std::make_unsigned_t<T>, std::make_unsigned_t<U>>::type;
		return static_cast<C>(lhs) == static_cast<C>(rhs);
	}

	// returns true if the any of the same bits are set
	// a & b != 0
	template <typename T, typename U>
	inline bool BitTest(const T lhs, const U bits)
	{
		// note: we use the common unsigned type in order to ensure no sign extension is performed anywhere!
		using C = typename std::common_type<std::make_unsigned_t<T>, std::make_unsigned_t<U>>::type;
		return (static_cast<C>(lhs) & static_cast<C>(bits)) != static_cast<C>(0);
	}

	// returns true if the bits are identical within the mask
	// a & mask == b & mask
	template <typename T, typename U, typename V>
	inline bool BitCompare(T lhs, U rhs, V mask)
	{
		// note: we use the common unsigned type in order to ensure no sign extension is performed anywhere!
		using C = typename std::common_type<std::make_unsigned_t<T>, std::make_unsigned_t<U>, std::make_unsigned_t<V>>::type;
		return (static_cast<C>(lhs) & static_cast<C>(mask)) == (static_cast<C>(rhs) & static_cast<C>(mask));
	}

	// returns true if the any of the same bits are set within the mask space
	// a & b & mask != 0
	template <typename T, typename U, typename V>
	inline bool BitTest(const T lhs, const U bits, const V mask)
	{
		// note: we use the common unsigned type in order to ensure no sign extension is performed anywhere!
		using C = typename std::common_type<std::make_unsigned_t<T>, std::make_unsigned_t<U>>::type;
		return (static_cast<C>(lhs) & static_cast<C>(bits) & static_cast<C>(mask)) != static_cast<C>(0);
	}

	// returns true if all of the bits are set in lhs that are set in rhs for the masked bits
	// a & mask == b & mask
	template <typename T, typename U, typename V> bool BitTestEqual(T lhs, U rhs, V mask) { return BitCompare(lhs, rhs, mask); }

	// returns true if all of the specified bits are set
	// a & b == b
	template <typename T, typename U>
	inline bool BitTestAll(const T lhs, const U bits)
	{
		// note: we use the common unsigned type in order to ensure no sign extension is performed anywhere!
		using C = typename std::common_type<std::make_unsigned_t<T>, std::make_unsigned_t<U>>::type;
		return (static_cast<C>(lhs) & static_cast<C>(bits)) == static_cast<C>(bits);
	}

	// clears the specified bits
	// a &= ~b
	template <typename T, typename U>
	inline void BitClear(T & lhs, const U bits)
	{
		// note: we cast to unsigned of T because T must store the results, but we want to be certain that no sign-extension is performed!
		using C = std::make_unsigned_t<T>;
		lhs &= ~static_cast<C>(bits);
	}

	// sets the specified bits
	// a |= b
	template <typename T, typename U>
	inline void BitSet(T & lhs, const U bits)
	{
		// note: we cast to unsigned of T because T must store the results, but we want to be certain that no sign-extension is performed!
		using C = std::make_unsigned_t<T>;
		lhs |= static_cast<C>(bits);
	}

	// sets sub-range of bits to a given value
	// a = a & ~mask | b & mask
	template <typename T, typename U, typename V>
	inline void BitSetTo(T & lhs, const U mask, const V bits)
	{
		// note: we cast to unsigned of T because T must store the results, but we want to be certain that no sign-extension is performed!
		using C = std::make_unsigned_t<T>;
		lhs = (lhs & ~static_cast<C>(mask)) | (static_cast<C>(bits) & static_cast<C>(mask));
	}

	// sets or clears the specified bits depending on the given conditional value
	// condition ? a |= b : a &= ~b
	template <typename T, typename U, typename V>
	inline void BitSetIf(T & lhs, const U bits, V cond)
	{
		// note: we cast to unsigned of T because T must store the results, but we want to be certain that no sign-extension is performed!
		cond ? BitSet(lhs, bits) : BitClear(lhs, bits);
	}

}