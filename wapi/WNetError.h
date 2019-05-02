#pragma once

#include "WinAPIError.h"

// windows networking library (needed for WNet related stuff)
#pragma comment(lib, "mpr.lib")

namespace tbx::wapi {

	// returns an error message for a WinAPI error and includes the named API and returns the extended error code and provider string
	std::string GetWNetErrorMessage(const std::string & api, const DWORD dwError, DWORD & dwExtendedError, std::string & provider);

	inline std::string GetWNetErrorMessage(const std::wstring & api, const DWORD dwError, DWORD & dwExtendedError, std::string & provider) 
	{
		return GetWNetErrorMessage(narrow(api), dwError, dwExtendedError, provider);
	}

	class CWNetAPIErrorException : public CContextException
	{
		DWORD		error;		// Primary error code
		DWORD		extended;	// WNet extended error code (if any)
		std::string provider;	// WNet provider (if any)
	public:
		CWNetAPIErrorException(const char * source, const char * api, DWORD error)
			: CContextException(source, GetWNetErrorMessage(api, error, extended, provider))
			, error(error)
			, extended(extended)
			, provider(provider)
		{ }

		CWNetAPIErrorException(const wchar_t * source, const char * api, DWORD error)
			: CContextException(source, GetWNetErrorMessage(api, error, extended, provider))
			, error(error)
			, extended(extended)
			, provider(provider)
		{ }

		CWNetAPIErrorException(const char * source, const wchar_t * api, DWORD error)
			: CContextException(source, GetWNetErrorMessage(api, error, extended, provider))
			, error(error)
			, extended(extended)
			, provider(provider)
		{ }

		CWNetAPIErrorException(const wchar_t * source, const wchar_t * api, DWORD error)
			: CContextException(source, GetWNetErrorMessage(api, error, extended, provider))
			, error(error)
			, extended(extended)
			, provider(provider)
		{ }

		DWORD GetError() const { return error; }
		DWORD GetExtendedError() const { return extended; }
		const std::string & GetProvider() const { return provider; }
	};

}
