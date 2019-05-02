// coerce.h
//
//	data type coercions with runtime exceptions when a coercion fails
//	no strings here, no complex types, just the basic machinery to be extended elsewhere to avoid dependencies

#pragma once

#include <stdexcept>		// std::exception

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// convert_to<R,T>
	//
	// Allows for partial or full specializations for type conversions
	//
	// Notes:
	//	- typedefs are not equal, so a partial specialization of WORD is not the same as unsigned __int16
	//  - T* -> bool : is potentially problematical, so no partial specialization is supplied
	//                 however, it should still result in the bool partial specialization, below, 
	//                 but can be specialized more by client code
	//////////////////////////////////////////////////////////////////////////

	// core functionality of converting a T to an R is simply static_cast<R>(T) unless R is an enum, in which case we go T -> underlying type of R -> R
	template <typename R, typename T>
	struct convert_to
	{
 		R operator () (T value)
		{
			if constexpr (std::is_enum<R>::value)
				return static_cast<R>(convert_to<std::underlying_type_t<R>, T>()(value));	// force an enumeration to convert viz its underlying type
			else
				return static_cast<R>(value);
		}
	};

	// T -> bool partial specialization (this defeats the inefficiancy warning of converting a non-bool to a bool, which shouldn't be a real optimization issue for any modern compiler)
	template <typename T> struct convert_to<bool, T> { bool operator () (T value) { return value ? true : false; } };


	/////////////////////////////////////////////////////////////////////////////////////////////////
	// As<type>(value) free functions
	/////////////////////////////////////////////////////////////////////////////////////////////////


	// attempts to instantiate a convert_to<> specialization to achieve the conversion
	// this allows convert_to to be specialized on T & R to achieve whatever results you want (e.g. string <-> integral)
	template <typename R, typename T> inline R As(T value) { return convert_to<R, T>()(value); }

	// named instances
	template <typename T> inline auto AsBool(T value) { return As<bool>(value); }
	template <typename T> inline auto AsShort(T value) { return As<short>(value); }
	template <typename T> inline auto AsInt(T value) { return As<int>(value); }
	template <typename T> inline auto AsLong(T value) { return As<long>(value); }
	template <typename T> inline auto AsFloat(T value) { return As<float>(value); }
	template <typename T> inline auto AsDouble(T value) { return As<double>(value); }

	// fixed width scalars (since C++ 11)
	template <typename T> inline auto AsByte(T value) { return As<uint8_t>(value); }
	template <typename T> inline auto AsWord(T value) { return As<uint16_t>(value); }
	template <typename T> inline auto AsDWord(T value) { return As<uint32_t>(value); }
	template <typename T> inline auto AsQWord(T value) { return As<uint64_t>(value); }

	/////////////////////////////////////////////////////////////////////////////////////////////////
	// Type assignment conversion deduction
	// Assign(target, value) -> target = As<declspec(target)>(value);
	/////////////////////////////////////////////////////////////////////////////////////////////////


	// e.g.: if (Assign(count, pszInputStreamCurrent)) ...
	// NOTE: this is a kludge!  Not all conversions set _errno, so not all conversion errors will be indicated by the return value.  
	//       However, all of the atoX functions do set _errno, so for the typical case this should be fine.
	// TODO: clean up client code to not rely upon this!  Maybe have a different name for string <-> value assignments that do use errno?
	template <typename R, typename T> inline bool Assign(R & result, T source) { errno = 0; result = convert_to<R, T>()(source); return errno == 0; }


	//////////////////////////////////////////////////////////////////////////
	// pointer <-> unsigned integral conversions
	// this is very similar to As<> and convert_to<> machinery, except that it is intended to be less general, 
	// and more focused on only preserving bits, and this is only applicable to simple types (including standard layout pointers)
	//////////////////////////////////////////////////////////////////////////

	// integral_from_ptr<type>::stype -> signed integral bitwise equivalent
	// integral_from_ptr<type>::utype -> unsigned integral bitwise equivalent
	template<class T>
	struct integral_from_ptr
	{
		static_assert(std::is_pointer_v<T>);

		using stype =
			std::conditional_t<sizeof(T) == 1, int8_t,
			std::conditional_t<sizeof(T) == 2, int16_t,
			std::conditional_t<sizeof(T) == 4, int32_t,
			std::conditional_t<sizeof(T) == 8, int64_t,
			void
			>>>;

		using utype =
			std::conditional_t<sizeof(T) == 1, uint8_t,
			std::conditional_t<sizeof(T) == 2, uint16_t,
			std::conditional_t<sizeof(T) == 4, uint32_t,
			std::conditional_t<sizeof(T) == 8, uint64_t,
			void
			>>>;
	};

	// convenience for pointer to scalar conversions
	template<typename T> using unsigned_from_ptr_t = typename integral_from_ptr<T>::utype;
	template<typename T> using signed_from_ptr_t = typename integral_from_ptr<T>::stype;

	// make value unsigned
	// this works for integrals and pointers, resulting in a same width unsigned integral in both cases
	template<class T>
	constexpr auto make_unsigned_value(T value)
	{
		if constexpr (std::is_pointer_v<T>)
			return reinterpret_cast<unsigned_from_ptr_t<T>>(value);
		else
			return static_cast<std::make_unsigned_t<T>>(value);
	}

	//////////////////////////////////////////////////////////////////////////
	// type coercion
	// similar to convert_to and As<T>, but with runtime overflow checking
	// used extensively in x64 WinAPI wrappers due to 32bit interfaces being incongruous 
	// with pointer arithmetic and std library collection size() members and operators
	//////////////////////////////////////////////////////////////////////////

	// coercion failure exception
	// this is thrown where a coercion results in data loss
	class coercion_failure : public std::overflow_error
	{
	public:
		typedef std::overflow_error _Mybase;

		// construct from message string
		explicit coercion_failure(const std::string & _Message) : _Mybase(_Message.c_str()) { }
		explicit coercion_failure(const char *_Message) : _Mybase(_Message) { }
		coercion_failure() : _Mybase("runtime error: coercion data overflow") { }
	};

	template <typename R, typename T>
	struct coercer
	{
		// coerce is very simple: just retain bits
		R operator () (T value)
		{
			// coercion is only for scalar types (arithmetic and pointers)
			static_assert(std::is_scalar_v<R>);
			static_assert(std::is_scalar_v<T>);

			// this could be a serious performance issue - we might want to allow for an unchecked variation 
			// (of course the user could just bypass coercion and use reinterpret or static casts to avoid runtime checks!)
			// still - we might want to offer an unchecked variant that only throws in debug builds
			if (static_cast<R>(value) != value)
				throw coercion_failure();

			return static_cast<R>(value);
		}
	};

	// partial specialization on pointer to pointer coercion
	// here we rely on reinterpret_cast instead of static_cast to allow us to be more flexible (so long as we don't lose any significant bits!)
	template <typename R, typename T>
	struct coercer<R*, T*>
	{
		// coerce is very simple: just retain bits
		R * operator () (T * value)
		{
			if (reinterpret_cast<T*>(reinterpret_cast<R*>(value)) != value)
				throw coercion_failure();
			return reinterpret_cast<R*>(value);
		}
	};

	// partial specialization on pointer to pointer coercion
	// here we rely on reinterpret_cast instead of static_cast to allow us to be more flexible (so long as we don't lose any significant bits!)
	template <typename R, typename T>
	struct coercer<R, T*>
	{
		// coerce from a pointer to a scalar
		R operator () (T * value)
		{
			using uptr_t = unsigned_from_ptr_t<T*>;

			// first cast the pointer to a scalar
			auto vprime = (uptr_t)(value);
			auto vfinal = (R)vprime;

			// check round-trip
			if ((T*)(uptr_t)(vfinal) != value)
				throw coercion_failure();

			// we're good
			return vfinal;
		}
	};

	// partial specialization on pointer to pointer coercion
	// here we rely on reinterpret_cast instead of static_cast to allow us to be more flexible (so long as we don't lose any significant bits!)
	template <typename R, typename T>
	struct coercer<R*, T>
	{
		// coerce from a scalar to a pointer
		R * operator () (T value)
		{
			using uptr_t = unsigned_from_ptr_t<R*>;

			// first cast the value to an unsigned pointer scalar equivalent
			auto vprime = (uptr_t)(value);
			auto vfinal = (R*)vprime;

			// check round-trip
			if ((T)(uptr_t)(vfinal) != value)
				throw coercion_failure();

			// we're good
			return vfinal;
		}
	};

	// usage: coerce<DWORD>(sizeof(T))
	// this throws coercion_failure if the value doesn't fit at runtime
	template <typename R, typename T> auto coerce(T value) { return coercer<R,T>()(value); }

	// usage: coerce_assign(target, source) -> target = coerce<declspec(target)>(source);
	// this throws coercion_failure if the value doesn't fit at runtime
	template <typename R, typename T> auto coerce_assign(R & target, T value) { target = coercer<R, T>()(value); }
}
