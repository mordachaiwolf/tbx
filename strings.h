#pragma once

#include <algorithm> 
#include <cctype>
#include <locale>

namespace tbx {

	// trim from start (in place)
	inline void trim_left(std::string & s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) { return !std::isspace(ch); }));
	}

	// trim from end (in place)
	inline void trim_right(std::string & s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) { return !std::isspace(ch); }).base(), s.end());
	}

	// trim from both ends (in place)
	inline void trim(std::string & s) {
		trim_right(s);
		trim_left(s);
	}

}
