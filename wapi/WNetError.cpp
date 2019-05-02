#include "stdafx.h"
#include "WNetError.h"

namespace tbx::wapi {

	std::string GetWNetErrorMessage(const std::string & api, const DWORD dwError, DWORD & dwExtendedError, std::string & provider)
	{
		// initialize the out variables
		dwExtendedError = NO_ERROR;
		provider = "";

		// get the base error explanation
		std::string what = GetWinAPIErrorMessage(api, dwError);

		// get extra if this is an extended error
		if (dwError == ERROR_EXTENDED_ERROR)
		{
			// docs indicate both of these strings will be truncated but null terminated if buffer is too small...
			wchar_t wszExplain[512];	// arbitrary - this is listed as "256 recommended" in MSDN docs.  
			wchar_t wszProvider[128];	// not specified in docs, but I assume shorter 

			const DWORD dwExtraError = WNetGetLastError(&dwExtendedError, wszExplain, static_cast<int>(count(wszExplain)), wszProvider, static_cast<int>(count(wszProvider)));
			if (dwExtraError == NO_ERROR)
			{
				provider = narrow(wszProvider);
				what += " [" + provider + " failed with WNet extended error: " + narrow(wszExplain) + "]";
			}
			else
				what += " [Unable to retrieve WNet extended error: " + GetWinAPIErrorMessage(dwExtraError) + "]";
		}

		return what;
	}

}
