#include "stdafx.h"
#include "strings.h"
#include "AutoStringBuffer.h"
#include "CustomException.h"

#include <stdarg.h>

namespace tbx {

	std::string format_string(const char * format, ...)
	{
		va_list args;
		va_start(args, format);

		std::string str;

		// attempt using a default buffer
		AutoStrBuffer buff(str, 63);
		auto required_length = std::vsnprintf(buff.get(), buff.size(), format, args);
		if (required_length > buff.length())
		{
			// reattempt using a larger buffer
			buff.resize(required_length);
			TBX_VERIFY(std::vsnprintf(buff.get(), buff.size(), format, args) == required_length);
		}

		// subtle: we're returning str, which will be set to contain the real value because it will be 
		//         moved out of buff when ~buff(), before moving str to caller
		return str;
	}

}
