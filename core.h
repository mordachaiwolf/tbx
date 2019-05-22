#pragma once

#include <initializer_list>

// NOTE: KEEP THIS FILE SIMPLE AND WITHOUT DEPENDENCIES!!!

//////////////////////////////////////////////////////////////////////////
// 
// SFINAE primer
// 
//////////////////////////////////////////////////////////////////////////
//
// Functions can trigger SFINAE by:
//	* return type
//	* auto -> return type
//	* defaulted function arguments
//  * defaulted template arguments
// 
//	template <typename T> std::enable_if_t<is_integral<T>, T> f(T value) { return value; }
// 
//	template <typename T> auto g(T value) -> decltype(T::size()) { return value.size(); }
// 
//	template <typename T> bool h(T value, typename std::enable_if<is_character<T>::value>::type* = 0) { return value & 0x3f; }
//
//	template <typename T, typename std::enable_if_t<is_character<T>::value, int> = 0> i(T c) { return c != 0; }
//	note: it must use the type of the enable_if_t and set it to a value, as above, to force it to be a distinguishing part of the signature of the function
//		  see notes under: https://en.cppreference.com/w/cpp/types/enable_if
// 
//////////////////////////////////////////////////////////////////////////
//
// structs are usually better off using (partial) template specialization or trait meta classes
//	* my_trait_class<T>::stuff_i_need_discrimated_by_T (using template specialization for the correct my_trait_class<>)
//	* template <> struct foo<bar> { /* foo specialized for bar */ };
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// preprocessor junk
//////////////////////////////////////////////////////////////////////////

#ifndef _STRINGIZE
#define _STRINGIZEX(x) #x
#define _STRINGIZE(x) _STRINGIZEX(x)
#endif

// MESSAGE(message) macro outputs "file (line) : message"
// In VisualStudio, this allows you to double click the output window and jump to the code
// usage: #pragma MESSAGE("double check this code!")
#define MESSAGE(message) __FILE__ "(" _STRINGIZE(__LINE__) ") : " message

//////////////////////////////////////////////////////////////////////////
// countof(x)		same as sizeof(array)/sizeof(*array)
//                  or collection.size()
//
// lengthof(x)		same as countof(array)-1
//
// NOTE: for these countof() functions and how to enhance them if we run into ambiguity, see:
//       https://stackoverflow.com/questions/48648148/how-to-write-a-size-function-that-works-on-any-type-of-collection-objects
//
// reminder to self: size(x) is a terrible name:
//	1) classes often have a member by that name, and that blocks any free size() function references (not a fan of this c++ limitation)
//	2) similarly, size is commonly used local variable name, which also interferes with name resolution to a free function by that name
//////////////////////////////////////////////////////////////////////////

namespace tbx
{
	// extract the count of items from a static array
	template <typename T, size_t size> size_t countof(const T(&collection)[size]) { return size; }

	// extract the count of items from a collection that provides a size() member
	template <typename T> auto countof(const T & collection) -> decltype(collection.size()) { return collection.size(); }

	// extract the maximum length of a fixed character string array
	// WARNING: this gives the maximum length string that can fit in the array  (i.e. countof(array) - 1)
	//			it does not actually do a strlen() on the array!
	template <size_t size> size_t lengthof(const char(&collection)[size]) { return size - 1; }
	template <size_t size> size_t lengthof(const wchar_t(&collection)[size]) { return size - 1; }
#ifdef char8_t
	template <size_t size> size_t lengthof(const char8_t(&collection)[size]) { return size - 1; }
#endif
	template <size_t size> size_t lengthof(const char16_t(&collection)[size]) { return size - 1; }
	template <size_t size> size_t lengthof(const char32_t(&collection)[size]) { return size - 1; }
}


namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// identity_of can help the compiler to access dependent types (for compilers that sometimes fail to get this right)
	// e.g.: tbx::identity<decltype(my_map)>::type::value_type
	//
	// This little gem allows you to force a template function to use only some
	// of the arguments for type resolution, and to force one or more other args
	// to resolve to that of a previous argument
	//////////////////////////////////////////////////////////////////////////
	template <typename T>
	struct identity_of
	{
		using type = T;
	};


	//////////////////////////////////////////////////////////////////////////
	// rank<n>
	// In order to allow us to rank two or more otherwise ambiguous implementations, we can use a ranking system to select the best workable approach
	// This relies on SFINAE
	// see: https://stackoverflow.com/questions/48648148/how-to-write-a-size-function-that-works-on-any-type-of-collection-objects
	// 
	// example:
	// 	auto convert_(T value, rank<2>) ... // highest rank, will try this first, but rank<2> will auto convert to a rank<1> if this fails to compile
	// 	auto convert_(T value, rank<1>) ... // middle rank, will try this next, but rank<1> will auto convert to a rank<0> if this fails to compile
	//	auto convert_(T value, rank<0>) ... // lowest rank, last stop is here

	namespace details {
		// rank<n> is a subclass of rank<n-1> and hence auto-convertible
		template <int n> struct rank : rank<n - 1> { };
		template <>      struct rank<0> { };
	}

	//////////////////////////////////////////////////////////////////////////
	// dereference_less is a simple predicate that returns "less" applied to the underlying object instances
	// for example:
	// template <typename T> using SetOfShared = std::multiset<std::shared_ptr<T>, tbx::dereference_less<std::shared_ptr<T>>>;

	template <typename pointer_t>
	struct dereference_less
	{
		constexpr bool operator()(const pointer_t & lhs, const pointer_t & rhs) const
		{
			return *lhs < *rhs;
		}
	};

}

// forces the template selection/matching algorithm to not deduce T from the specified argument
// usage: template <typename T> void myfun(T arg1, IDENTITYOF(T) arg2, ...);
#define IDENTITYOF(T) typename tbx::identity_of<T>::type

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// isoneof tests whether the initial argument value occurs in the given set of other values

	template <typename T, typename U>
	bool isoneof(T v, U v1) { return v == v1; }

	template <typename T, typename U, typename... Args>
	bool isoneof(T v, U v1, Args ... others) { return isoneof(v, v1) || isoneof(v, others...); }

	template <typename T, typename U>
	bool isoneof(T value, std::initializer_list<U> values)
	{
		for (const auto & e : values)
			if (value == e)
				return true;
		return false;
	}

	template <typename T, typename U, size_t size>
	bool isoneof(T value, const U(&arr)[size])
	{
		for (size_t i = 0; i < size; ++i)
			if (value == arr[i])
				return true;
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// is_one_of tests whether the type is one of a set of given types

	template<typename T, typename ... Ts>
	struct is_one_of : std::bool_constant<(std::is_same_v<T, Ts> || ...)> { };

	template <typename T, typename ... Ts> constexpr bool is_one_of_v = is_one_of<T, Ts...>::value;

}
