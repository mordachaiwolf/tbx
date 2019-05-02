#include "StdAfx.h"

#include "alphanum sort.h"
#include "SmartChar.h"

#if _AFX // MFC only
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#endif

namespace Toolbox {

	template <typename ctype, bool case_sensitive>
	int alphanumcmp(const ctype * pszLeft, const ctype * pszRight)
	{
		enum { STRING, NUMBER } mode = STRING;

		while (*pszLeft && *pszRight)
		{
			if (mode == STRING)
			{
				for (auto left = *pszLeft, right = *pszRight; left && right; left = *pszLeft, right = *pszRight)
				{
					// check if this are digit characters
					const auto digits = (is_digit(left) ? 0x2 : 0x0) | (is_digit(right) ? 0x1 : 0x0);

					// if both characters are digits, we continue in NUMBER mode
					if (digits == 0x3)
					{
						mode = NUMBER;
						break;
					}
					else if (digits == 0x2)
					{
						// numbers before strings
						return -1;
					}
					else if (digits == 0x1)
					{
						// strings after numbers
						return +1;
					}
					else
					{
						// neither is a digit

						// TODO: check if this is a lead-byte, do better comparison, skip trail byte(s)

						// compute the difference of both characters
						if (const auto diff = (case_sensitive ? left - right : _tolower(left) - _tolower(right)))
							return diff < 0 ? -1 : 1;

						// otherwise process the next characters
						++pszLeft;
						++pszRight;
					}
				}
			}
			else // mode==NUMBER
			{
				// get the left number
				unsigned long left = 0;
				while (*pszLeft && is_digit(*pszLeft))
				{
					// TODO: this can overflow
					left = left * 10 + *pszLeft - '0';
					++pszLeft;
				}

				// get the right number
				unsigned long right = 0;
				while (*pszRight && is_digit(*pszRight))
				{
					// TODO: this can overflow
					right = right * 10 + *pszRight - '0';
					++pszRight;
				}

				// if the difference is not equal to zero, we have a comparison result
				if (const auto diff = left - right)
					return diff < 0 ? -1 : 1;

				// otherwise we process the next substring in STRING mode
				mode = STRING;
			}
		}

		if (*pszRight)
			return -1;

		if (*pszLeft)
			return +1;

		return 0;
	}

	int alphanumcmp(const char * pszLeft, const char * pszRight)
	{
		return alphanumcmp<char, true>(pszLeft, pszRight);
	}

	int alphanumcmp(const wchar_t * pszLeft, const wchar_t * pszRight)
	{
		return alphanumcmp<wchar_t, true>(pszLeft, pszRight);
	}

	int alphanumcmpi(const char * pszLeft, const char * pszRight)
	{
		return alphanumcmp<char, false>(pszLeft, pszRight);
	}

	int alphanumcmpi(const wchar_t * pszLeft, const wchar_t * pszRight)
	{
		return alphanumcmp<wchar_t, false>(pszLeft, pszRight);
	}

}