#include "stdafx.h"
#include "WinAPIError.h"

#include "tbx/core.h"
#include "tbx/strings.h"

namespace tbx::wapi {

	// returns an error message for a WinAPI error (q.v. GetLastError())
	std::string GetWinAPIErrorMessage(const DWORD dwError)
	{
		// ask windows to do the real work
		TCHAR * lpMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // use default language
			(TCHAR *)&lpMsgBuf,
			0,
			nullptr
		);

		// copy the system string into the std::string (and convert to utf8)
		std::string message = narrow(lpMsgBuf);

		// free the system allocated buffer
		::LocalFree(lpMsgBuf);

		// remove the newline from the message string
		trim_right(message);

		// give it to the caller
		return message;
	}

	// returns an error message for a WinAPI error (q.v. GetLastError())
	std::string GetWinAPIErrorMessage(const DWORD dwError, const TCHAR * pszFirstArg, ...)
	{
		va_list arglist;
		va_start(arglist, dwError);

		// ask windows to do the real work
		TCHAR * lpMsgBuf;
		::FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			nullptr,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // use default language
			(TCHAR *)&lpMsgBuf,
			0,
			&arglist
		);

		va_end(arglist);

		// copy the system string into the std::string (and convert to utf8)
		std::string message = narrow(lpMsgBuf);

		// free the system allocated buffer
		::LocalFree(lpMsgBuf);

		// remove the newline from the message string
		trim_right(message);

		// give it to the caller
		return message;
	}

}
