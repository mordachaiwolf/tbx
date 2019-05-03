#pragma once

// NOTE: KEEP THIS FILE SIMPLE AND WITHOUT DEPENDENCIES!!!

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
	// literal_traits uses template specialization to achieve narrow or wide character literals for templates
	// the idea came from me (Steven S. Wolf), and the implementation from Mark Ransom:
	// q.v. http://stackoverflow.com/questions/4261673/templates-and-string-literals-and-unicode
	//////////////////////////////////////////////////////////////////////////
	template<typename T>
	struct literal_traits
	{
		using char_type = char;
		static const char * choose(const char * narrow, const wchar_t * wide) { return narrow; }
		static char choose(const char narrow, const wchar_t wide) { return narrow; }
	};

	template<>
	struct literal_traits<wchar_t>
	{
		using char_type = wchar_t;
		static const wchar_t * choose(const char * narrow, const wchar_t * wide) { return wide; }
		static wchar_t choose(const char narrow, const wchar_t wide) { return wide; }
	};

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

	// rank<n> is a subclass of rank<n-1> and hence auto-convertible
	template <int n> struct rank : rank<n - 1> { };
	template <>      struct rank<0> { };

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

// generates the narrow or wide character literal depending on T
// usage: LITERAL(char-type, "literal text") or LITERAL(char-type, 'c')
#define LITERAL(T,x) tbx::literal_traits<typename T>::choose(x, L##x)

// forces the template selection/matching algorithm to not deduce T from the specified argument
// usage: template <typename T> void myfun(T arg1, IDENTITYOF(T) arg2, ...);
#define IDENTITYOF(T) typename tbx::identity_of<T>::type
