#pragma once

#include <algorithm> 
#include <cctype>
#include <locale>

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
