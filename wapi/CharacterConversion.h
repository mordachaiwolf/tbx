#pragma once

#include "tbx\crt\AutoMalloc.h"
#include "WinAPIError.h"

// note: these throw CWinAPIException

namespace tbx::wapi {
	namespace CharacterConversion {

		// GetConversionLength returns the length (in target characters) for a conversion of the specified string (using the given code page)
		size_t GetConversionLength(const wchar_t * source, size_t cchLength, int codepage = CP_ACP);

		// GetConversionLength returns the length (in target characters) for a conversion of the specified string (using the given code page)
		size_t GetConversionLength(const char * source, size_t cchLength, int codepage = CP_ACP);

		// MakeConversionBuffer creates a buffer containing the converted string (optionally you may specify the replacement char and a bool to hold whether or not the replacement char was used or not)
		AutoMalloc<char> MakeConversionBuffer(const wchar_t * source, size_t cchLength);

		// MakeConversionBuffer creates a buffer containing the converted string (optionally you may specify the replacement char and a bool to hold whether or not the replacement char was used or not)
		AutoMalloc<char> MakeConversionBuffer(const wchar_t * source, size_t cchLength, const char chReplacement, BOOL & bUsedReplacementChar);

		// MakeConversionBuffer creates a buffer containing the converted string
		AutoMalloc<wchar_t> MakeConversionBuffer(const char * source, size_t cchLength);
	}

} // tbx