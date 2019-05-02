//
// adapted from http://www.davekoelle.com/alphanum.html
//
// NOTE!!! Windows supplies StrCmpLogicalW() which does it better?

#pragma once

namespace Toolbox {


	// returns the usual strcmp/strcmpi results of -1, 0, or 1
	int alphanumcmp(const char * pszLeft, const char * pszRight);
	int alphanumcmp(const wchar_t * pszLeft, const wchar_t * pszRight);
	int alphanumcmpi(const char * pszLeft, const char * pszRight);
	int alphanumcmpi(const wchar_t * pszLeft, const wchar_t * pszRight);

}