#pragma once

#include <algorithm> 
#include <cctype>
#include <locale>

//////////////////////////////////////////////////////////////////////////
// Character & String primitives
//
// WARNING!
//
//	character case and collation is complex and should be handled by a 
//	dedicated library that is written to support language/locale specific
//	algorithms.
//
//	this library assumes that you're using it to manipulate English strings
//	which is very often the case for reading and writing keywords that are
//	a part of the meta data or data stream structure itself, such as JSON
//	or XML or HTML or an INI file and etc.
//
//	we do NOT deal with locale here.  that is NOT what this library is for.
//	when you need locale awareness and smarter string handling which knows
//	how to collate in French or Traditional Chinese, you will need to write
//	your code using another library which has those smarts, assuming you
//	genuinely need them (user data manipulation and presentation often do,
//	whereas meta data and data file manipulation almost certainly do not).
//
// references:
//	http://site.icu-project.org/
//	http://utf8everywhere.org/

namespace tbx {

	using strcmpcode = int;	// < 0 = less than, 0 = equal, > 0 = greater than

	inline bool is_lt(strcmpcode strcmp_result) { return strcmp_result <= -1; }
	inline bool is_eq(strcmpcode strcmp_result) { return strcmp_result == 0; }
	inline bool is_gt(strcmpcode strcmp_result) { return strcmp_result >= 1; }

	// internal backwards compatibility shims
	// TODO: remove these!
#define STRCMP_LESS(strcmp_result) is_lt(strcmp_result)
#define STRCMP_EQUAL(strcmp_result) is_eq(strcmp_result)
#define STRCMP_GREATER(strcmp_result) is_gt(strcmp_result)

#ifdef char8_t
	template <typename T>
	struct is_character : is_one_of<T, char, wchar_t, char8_t, char16_t, char32_t> {};
#else
	template <typename T>
	struct is_character : is_one_of<T, char, wchar_t, char16_t, char32_t> {};
#endif

	template <typename T>
	constexpr bool is_character_v = is_character<T>::value;

	//////////////////////////////////////////////////////////////////////////
	// literal_traits uses template specialization to achieve narrow or wide character literals for templates
	// the idea came from me (Steven S. Wolf), and the implementation from Mark Ransom:
	// q.v. http://stackoverflow.com/questions/4261673/templates-and-string-literals-and-unicode
	//////////////////////////////////////////////////////////////////////////

#ifdef char8_t
	using CHAR8_T = char8_t;
#else
	using CHAR8_T = char;
#endif

	namespace details {

		// q.v. https://stackoverflow.com/questions/56049410/c-how-can-i-improve-this-bit-of-template-meta-program-to-give-back-the-array-i#56049702
		// use recursion list expansion to select the matching type
		// use decltype(auto) to return the type without decaying (so it is still a const ctype(&)[size])
		template<typename T, typename C, size_t N, typename... Ts>
		constexpr decltype(auto) choose_literal(const C(&result)[N], Ts const&... rest) {
			if constexpr (std::is_same_v<T, C>)
				return result;
			else
				return choose_literal<T>(rest...);
		}

		// this overload gives back a single character
		template<typename T, typename C, typename... Ts>
		constexpr T choose_literal(C result, Ts... rest) {
			if constexpr (std::is_same_v<T, C>)
				return result;
			else
				return choose_literal<T>(rest...);
		}
	}

	// generates the appropriate character literal using preprocessor voodoo
	// usage: LITERAL(char-type, "literal text") or LITERAL(char-type, 'c')
#define LITERAL(T,x) details::choose_literal<T>(x, L##x, u8##x, u##x, U##x)


	//////////////////////////////////////////////////////////////////////////
	// English-only character classification (q.v. classic locale)

	template <typename T>
	bool is_control(const T & ch) { return ch < 0x20 || ch == 0x7F; }
	template <typename T>
	bool is_printable(const T & ch) { return !is_control(ch); }
	template <typename T>
	bool is_whitespace(const T & ch) { return ch == 0x20 || (ch >= 0x09 && ch <= 0x0D); }
	template <typename T>
	bool is_lowercase(const T & ch) { return ch >= LITERAL(T, 'a') && ch <= LITERAL(T, 'z'); }
	template <typename T>
	bool is_uppercase(const T & ch) { return ch >= LITERAL(T, 'A') && ch <= LITERAL(T, 'Z'); }
	template <typename T>
	bool is_alphabetical(const T & ch) { return is_lowercase(ch) || is_uppercase(ch); }
	template <typename T>
	bool is_digit(const T & ch) { return ch >= LITERAL(T, '0') && ch <= LITERAL(T, '9'); }
	template <typename T>
	bool is_alphanumeric(const T & ch) { return is_alphabetical(ch) || is_digit(ch); }
	template <typename T>
	bool is_hex_digit(const T & ch) { return is_digit(ch) || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'); }

	//////////////////////////////////////////////////////////////////////////
	// English-only case manipulation

	template <typename T>
	T az_upper(T ch) { return is_lowercase(ch) ? ch & ~0x20 : ch; }

	template <typename T>
	T az_lower(T ch) { return is_uppercase(ch) ? ch | 0x20 : ch; }

	//////////////////////////////////////////////////////////////////////////
	// get_string
	// returns the `const char *` for a given string-type
	//TODO: We should probably be replacing all such code with std::string_view...

	// known array of characters
	template <
		typename T,
		typename std::enable_if_t<std::is_array_v<T>, std::size_t> size = std::extent_v<T>
	>
	decltype(auto) get_string(T & str) { return str; }
	
	// decayed pointer to a character array
	template <
		typename T,
		typename std::enable_if_t<std::is_pointer_v<T> && is_character_v<std::remove_pointer_t<T>>, std::size_t> = 0
	>
	decltype(auto) get_string(const T & str) { return str; }

	// any type that supports the member c_str()
	template <typename T> auto get_string(const T & str) -> decltype(str.c_str()) { return str.c_str(); }

	// any type that supports the member GetString()
	template <typename T> auto get_string(const T & str) -> decltype(str.GetString()) { return str.GetString(); }


	//////////////////////////////////////////////////////////////////////////
	// IsEmpty

	// return true if the given character array pointer is nullptr or points to an empty string
	template <typename T, typename std::enable_if_t<is_character_v<T>, int> = 0>
	bool IsEmpty(const T * psz) { return !psz || !psz[0]; }

	// return true if the given string or collection is empty
	template <typename T> auto IsEmpty(const T & collection) -> decltype(collection.empty()) { return collection.empty(); }

	// return true if the given string or collection is empty
	template <typename T> auto IsEmpty(const T & collection) -> decltype(collection.IsEmpty()) { return collection.IsEmpty(); }


	//////////////////////////////////////////////////////////////////////////
	// GetLength

	// return the length of the string with known size (upper bounds)
	template <
		typename T,
		typename std::enable_if_t<!std::is_array_v<T>, int> = 0,
		typename std::enable_if_t<is_character_v<T>, int> = 0
	>
	auto GetLength(const T * psz, size_t size) {
		size_t l = 0;
		if (psz && size)
			while (*psz)
			{
				if (++l == size)
					break;
				++psz;
			}
		return l;
	}

	// return the length of the string with known size (upper bounds)
	template <
		typename T,
		size_t size,
		typename std::enable_if_t<std::is_array_v<T>, int> = 0,
		typename std::enable_if_t<is_character_v<T>, int> = 0
	>
	auto GetLength(const T(&psz)[size]) { return GetLength(psz, size); }

	// return the length of the string
	// WARNING: prefer the bounded version, below!
	template <
		typename T,
		typename std::enable_if_t<!std::is_array_v<T>, int> = 0,
		typename std::enable_if_t<is_character_v<T>, int> = 0
	> 
	auto GetLength(const T * psz) {
		using namespace std;
		return psz ? char_traits<T>::length(psz) : 0; 
	}

	// return the length of the string-like entity
	template <typename T> auto GetLength(const T & collection) -> decltype(collection.length()) { return collection.length(); }

	// return the length of the string-like entity
	template <typename T> auto GetLength(const T & collection) -> decltype(collection.GetLength()) { return collection.GetLength(); }


	//////////////////////////////////////////////////////////////////////////
	// Blank C strings

	// returns an appropriate 
	template <typename T>
	constexpr const T * GetBlank() {
		return LITERAL(T, "");
	}

	// returns the given string, or the blank string if the pointer is null
	template <typename T>
	const T * StringOrBlank(const T * psz)  {
		return psz ? psz : GetBlank<T>();
	}

	// lexicographical comparison
	template <typename T>
	strcmpcode compare(const T * first, const T * second, size_t length) {
		return (strcmpcode)std::char_traits<T>::compare(first, second, length);
	}

	// lexicographical comparison
	template <typename T>
	strcmpcode compare(const T * first, const T * second) {
		const auto l1 = GetLength(first);
		const auto l2 = GetLength(second);
		auto result = compare(first, second, min(l1, l2));
		return result ? result : l1 < l2 ? -1 : l1 > l2 ? +1 : 0;
	}

	// case insensitive comparison
	// NOTE! this function only really makes sense for strings that are in English
	//       there are languages where case means something, but these functions aren't that sophisticated!
	//		 for non-English strings, please use another library such as ICU: http://site.icu-project.org/
	template <typename T>
	strcmpcode compare_no_case(const T * first, const T * second, size_t length) {
		while (length--)
		{
			auto ch1 = az_upper(*first);
			auto ch2 = az_upper(*second);
			if (ch1 != ch2)
				return ch1 < ch2 ? -1 : +1;
			++first;
			++second;
		}

		return 0;
	}

	template <typename T>
	strcmpcode compare_no_case(const T * first, const T * second) {
		const auto l1 = GetLength(first);
		const auto l2 = GetLength(second);
		auto result = compare_no_case(first, second, min(l1, l2));
		return result ? result : l1 < l2 ? -1 : l1 > l2 ? +1 : 0;
	}

	// copy source to dest, returns dest (UB for overlapping buffers)
	template <typename T>
	auto copy(T * dest, size_t size, const T * source) { return std::char_traits<T>::copy(dest, size, source); }

	// copy source to dest, returns dest (UB for overlapping buffers)
	template <typename T, size_t size>
	auto copy(T(&dest)[size], const T * source) { return copy(dest, size, source); }

	// partial copy
	template <typename T>
	auto copy(T * dest, size_t size, const T * source, size_t length) {
		if (size > length)
		{
			copy(dest, length, source);
			dest[length] = 0;
		}
		else
		{
			copy(dest, size - 1, source);
			dest[size - 1] = 0;
		}
		return dest;
	}

	template <typename T, size_t size>
	auto copy(T(&dest)[size], const T * source, size_t length) { return copy(dest, size, source, length); }

	// concatenation
	template <typename T>
	auto concatenate(T * dest, size_t size, const T * source) {
		auto offset = GetLength(dest, size);
		if (size > offset)
			return copy(dest + offset, size - offset, source);
		return dest;
	}

	template <typename T, size_t size>
	auto concatenate(T(&dest)[size], const T * source) { return concatenate(dest, size, source); }

	// partial concatenation
	template <typename T>
	auto concatenate(T * dest, size_t size, const T * source, size_t length) {
		auto offset = GetLength(dest, size);
		if (size > offset)
			copy(dest + offset, size - offset, source, length);
		return dest;
	}

	template <typename T, size_t size>
	auto concatenate(T(&dest)[size], const T * source, size_t length) { return concatenate(dest, size, source, length); }

	// uppercase conversion
	// NOTE! this function only really makes sense for strings that are in English
	//       there are languages where case means something, but these functions aren't that sophisticated!
	//		 for non-English strings, please use another library such as ICU: http://site.icu-project.org/
	template <typename T>
	auto make_uppercase(T * dest, size_t maxlength) {
		for (size_t i = 0; i < maxlength; ++i)
		{
			if (!dest[i])
				break;
			dest[i] = az_upper(dest[i]);
		}
		return dest;
	}

	template <typename T, size_t size>
	auto make_uppercase(T(&dest)[size]) { return make_uppercase(dest, size); }

	// lowercase conversion
	// NOTE! this function only really makes sense for strings that are in English
	//       there are languages where case means something, but these functions aren't that sophisticated!
	//		 for non-English strings, please use another library such as ICU: http://site.icu-project.org/
	template <typename T>
	auto make_lowercase(T * dest, size_t maxlength) {
		for (size_t i = 0; i < maxlength; ++i)
		{
			if (!dest[i])
				break;
			dest[i] = az_lower(dest[i]);
		}
		return dest;
	}

	template <typename T, size_t size>
	auto make_lowercase(T(&dest)[size]) { return make_lowercase(dest, size); }

	// find character
	template <typename T>
	T * find(T * psz, size_t maxlength, T chr) {
		using namespace std;
		return char_traits<T>::find(psz, maxlength, chr);
	}

	template <typename T, size_t size>
	T * find(T (&psz)[size], T chr) { return find(psz, size, chr); }

	// reverse find character
	template <typename T>
	T * reverse_find(T * psz, size_t length, T chr) {
		while (length--)
			if (psz[length] == chr)
				return psz;
		return nullptr;
	}

	// search for substring
	template <typename T>
	T * find(T * psz, size_t size, const T * search, size_t search_length) 
	{
		// we can't find something larger than the target, in the target
		const auto target_length = GetLength(psz, size);
		if (search_length > target_length)
			return nullptr;

		// empty string always matches (so sayest the standard)
		if (search_length == 0)
			return psz;

		// find first char
		// check if it is a match on the rest
		// repeat until exhausted for length(search)
		const auto probe_end = psz + (target_length - search_length) + 1;
		for (auto probe = psz; ; ++probe)
		{
			// find next occurrence of first search char
			probe = find(probe, probe_end - probe, *search);

			// if we can't find the start, then it isn't here
			if (!probe)
				return nullptr;

			// check if we found match
			if (is_eq(compare(probe, search, search_length)))
				return probe;
		}
	}

	template <typename T>
	T * find(T * psz, size_t size, const T * search) { return find(psz, size, search, GetLength(search)); }

	template <typename T, size_t size>
	T * find(T (&psz)[size], const T * search) { return find(psz, size, search); }

	template <typename T, size_t size>
	T * find(T(&psz)[size], const T * search, size_t length) { return find(psz, size, search, length); }

	// reverse find substring
	template <typename T>
	T * reverse_find(T * psz, size_t size, const T * search, size_t search_length)
	{
		// we can't find something larger than the target, in the target
		const auto target_length = GetLength(psz, size);
		if (search_length > target_length)
			return nullptr;

		// empty string always matches (so sayest the standard)
		if (search_length == 0)
			return psz + target_length;

		// rfind first char
		// check if it is a match on the rest
		// repeat until exhausted for length(search)
		for (auto probe = psz + target_length - search_length; ; --probe)
		{
			// find next occurrence of first search char
			auto probe = reverse_find(psz, probe - psz, *search);

			// if we can't find the start, then it isn't here
			if (!probe)
				return nullptr;

			// check if we found match
			if (is_eq(compare(probe, search, search_length)))
				return probe;
		}
	}

#ifdef USE_LEGACY_FORMAT
	// formatted strings
	template <typename T, size_t size>
	int format(T(&psz)[size], const T * formatting, ...)
	{
		va_list args;
		va_start(args, formatting);
		return vsprintf_s(psz, size, formatting, args);
	}

	template <typename T>
	int format(T * psz, size_t size, const T * formatting, ...)
	{
		va_list args;
		va_start(args, formatting);
		return vsprintf_s(psz, size, formatting, args);
	}

	// formatting
	template <typename T, size_t size>
	int vformat(T(&psz)[size], const T * formatting, va_list args)
	{
		return vsprintf_s(psz, size, formatting, args);
	}

	template <typename T>
	int vformat(T * psz, size_t size, const T * formatting, va_list args)
	{
		return vsprintf_s(psz, size, formatting, args);
	}
#endif // USE_LEGACY_FORMAT

}

////////////////////////////////////////////////////////////////////////
// string accumulation
////////////////////////////////////////////////////////////////////////

namespace tbx {

	namespace overflow_policy {
		struct throw_range_error
		{
			static void handle_overflow(const char * context)
			{
				throw std::range_error(context);
			}
		};
	}

	template <typename T, size_t size, typename overflow_policy = overflow_policy::throw_range_error>
	struct fixed_character_back_insert_iterator
	{
		using char_t = T;
		typedef T container_type[size];

		// construct us on a fixed sized buffer
		fixed_character_back_insert_iterator(container_type &sz) : m_sz(sz), m_length(Toolbox::SmartChar::get_length(sz)) {}

		// nonstandard extension
		bool full() const { return m_length == size; }

		// what we actually do
		fixed_character_back_insert_iterator & operator = (T c)
		{
			if (full())
			{
				overflow_policy::handle_overflow(__FUNCTION__);
			}
			else
			{
				// we could optimize this by only setting the null marker upon deconstruction or similar, but that makes us much less safe...
				// hopefully the compiler can optimize this double write to adjacent address reasonably well...
				m_sz[m_length] = c;
				m_sz[++m_length] = 0;
			}
			return *this;
		}

		// these are all no-ops for back inserter's
		fixed_character_back_insert_iterator & operator * () { return *this; }
		fixed_character_back_insert_iterator & operator ++ () { return *this; }
		fixed_character_back_insert_iterator & operator ++ (int) { return *this; }

		// efficiency extension: insert a whole run of characters...
		fixed_character_back_insert_iterator & operator = (char_t * psz)
		{
			// check if we have that much available (incl. null terminator)
			const size_t length = Toolbox::SmartChar::get_length(psz) + 1;
			if (length > size - m_length)
				overflow_policy::handle_overflow(__FUNCTION__);
			else
			{
				Toolbox::SmartChar::copy_at(m_sz, m_length, psz, length);
				m_length += length;
			}
			return *this;
		}

	private:
		container_type &	m_sz;		// where we're accumulating our data
		size_t				m_length;	// our current null terminator is here...
	};

	template <typename char_t, size_t size, typename overflow_policy = overflow_policy::throw_range_error>
	fixed_character_back_insert_iterator<char_t, size, overflow_policy> make_string_back_inserter(char_t(&str)[size])
	{
		return fixed_character_back_insert_iterator<char_t, size, overflow_policy>(str);
	}

	// adapter to use a std::string for target of a back_insert_iterator (i.e. an output iterator)
	template <typename string_t>
	struct string_back_insert_iterator
	{
		using str_t = string_t;
		using char_t = typename string_t::traits_type::char_type;

		// construct us on a string
		string_back_insert_iterator(string_t & str) : m_string(str) {}

		// what we actually do
		string_back_insert_iterator & operator = (char_t c) { m_string += c; return *this; }

		// these are all no-ops for back inserter's
		string_back_insert_iterator & operator * () { return *this; }
		string_back_insert_iterator & operator ++ () { return *this; }
		string_back_insert_iterator & operator ++ (int) { return *this; }

		// efficiency extension: insert a whole run of characters...
		string_back_insert_iterator & operator = (const char_t * psz) { m_string += psz; return *this; }

	private:
		string_t &	m_string;	// where we're accumulating our data
	};

	template <typename string_t>
	string_back_insert_iterator<string_t> make_string_back_inserter(string_t & str)
	{
		return string_back_insert_iterator<string_t>(str);
	}
}

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// whitespace trimming for any string-like entity that supports the necessary ops

	// trim from start (in place)
	template <typename str_t>
	void trim_left(str_t & s) {
		using namespace std;
		s.erase(begin(s), find_if(begin(s), end(s), [](auto ch) { return !isspace(ch); }));
	}

	// trim from end (in place)
	template <typename str_t>
	void trim_right(str_t & s) {
		using namespace std;
		s.erase(find_if(rbegin(s), rend(s), [](auto ch) { return !isspace(ch); }).base(), end(s));
	}

	// trim from both ends (in place)
	template <typename str_t>
	void trim(str_t & s) {
		trim_right(s);
		trim_left(s);
	}

	//////////////////////////////////////////////////////////////////////////

	//TODO: convert C language ... with template driven decoding

	// printf style string formatting helpers
	std::string format_string(const char * format, ...);

}
