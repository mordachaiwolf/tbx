#pragma once

#include "tbx\CustomException.h"

// NOTE: Any WinAPI calls that happen before GetLastError() can *change* the current error code!!!
//       As a practical matter: a CStringW(const char * psz) ctor will overwrite the current error with 122: buffer too small!
//		 So it is paramount that you capture the current error first, then generate the strings for the error context second!

// NOTE: Chaining function calls doesn't guarantee evaluation ordering for the arguments - so it doesn't buy us anything in this context
//		 bottom line: CALLER MUST CAPTURE THE ERROR CODE, THEN SUPPLY THE CONTEXT, UNLESS THE CONTEXT IS 100% TRIVIAL!!!

namespace tbx::wapi {

	// returns an error message for a WinAPI error (q.v. GetLastError())
	std::string GetWinAPIErrorMessage(const DWORD dwError);

	// same as GetWinAPIErrorMessage, but allows the system to do %1 %2 Substitutions viz the additional arguments
	std::string GetWinAPIErrorMessage(const DWORD dwError, const TCHAR * pszFirstArg, ...);

	// same as GetWinAPIErrorMessage, plus includes the named API in the description (e.g. FindNextFile() : invalid argument)
	inline std::string GetWinAPIErrorMessage(const std::string & api, const DWORD dwError) { return api + "() : " + GetWinAPIErrorMessage(dwError); }

	// same as GetWinAPIErrorMessage, embeds the args string within the API call in the output
	inline std::string GetWinAPIErrorMessage(const std::string & api, const std::string & args, const DWORD dwError) { return api + "(" + args + ") : " + GetWinAPIErrorMessage(dwError); }

	class CWinAPIErrorException : public CContextException
	{
		DWORD	error;
	public:
		// source
		CWinAPIErrorException(const char * source, DWORD error) : CContextException(source, GetWinAPIErrorMessage(error)), error(error) {}
		CWinAPIErrorException(const wchar_t * source, DWORD error) : CContextException(source, GetWinAPIErrorMessage(error)), error(error) {}

		// source + api
		CWinAPIErrorException(const char * source, const char * api, DWORD error) : CContextException(source, GetWinAPIErrorMessage(api, error)), error(error) {}
		CWinAPIErrorException(const wchar_t * source, const char * api, DWORD error) : CContextException(source, GetWinAPIErrorMessage(api, error)), error(error) {}

		// source + api + args
		CWinAPIErrorException(const char * source, const char * api, const std::string & args, DWORD error) : CContextException(source, GetWinAPIErrorMessage(api, error)), error(error) {}
		CWinAPIErrorException(const wchar_t * source, const char * api, const std::string & args, DWORD error) : CContextException(source, GetWinAPIErrorMessage(api, error)), error(error) {}

		DWORD GetError() const { return error; }
	};

}