#include "stdafx.h"

#include "CharacterConversion.h"

namespace tbx::wapi {
	namespace CharacterConversion {

		size_t GetConversionLength(const wchar_t * source, size_t cchLength, int codepage /*= CP_ACP*/)
		{
			return static_cast<size_t>(WideCharToMultiByte(codepage, 0, source, static_cast<int>(cchLength), nullptr, 0, nullptr, nullptr));
		}

		size_t GetConversionLength(const char * source, size_t cchLength, int codepage /*= CP_ACP*/)
		{
			return static_cast<size_t>(MultiByteToWideChar(codepage, 0, source, static_cast<int>(cchLength), nullptr, 0));
		}

		AutoMalloc<char> MakeConversionBuffer(const wchar_t * source, size_t cchLength)
		{
			// avoid problems with empty string
			if (!cchLength)
				return AutoMalloc<char>(0);

			const size_t cchConversion = GetConversionLength(source, cchLength);
			if (!cchConversion)
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "WideCharToMultiByte", error);
			}
			AutoMalloc<char> buffer(cchConversion);
			if (!WideCharToMultiByte(CP_ACP, 0, source, static_cast<int>(cchLength), buffer, static_cast<int>(cchConversion), nullptr, nullptr))
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "WideCharToMultiByte", error);
			}
			return std::move(buffer);
		}

		AutoMalloc<char> MakeConversionBuffer(const wchar_t * source, size_t cchLength, const char chReplacement, BOOL & bUsedReplacementChar)
		{
			// avoid problems with empty string
			if (!cchLength)
				return AutoMalloc<char>(0);

			const size_t cchConversion = GetConversionLength(source, cchLength);
			if (!cchConversion)
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "WideCharToMultiByte", error);
			}
			AutoMalloc<char> buffer(cchConversion);
			if (!WideCharToMultiByte(CP_ACP, 0, source, static_cast<int>(cchLength), buffer, static_cast<int>(cchConversion), &chReplacement, &bUsedReplacementChar))
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "WideCharToMultiByte", error);
			}
			return std::move(buffer);
		}

		AutoMalloc<wchar_t> MakeConversionBuffer(const char * source, size_t cchLength)
		{
			// avoid problems with empty string
			if (!cchLength)
				return AutoMalloc<wchar_t>(0);

			const size_t cchConversion = GetConversionLength(source, cchLength);
			if (!cchConversion)
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "MultiByteToWideChar", error);
			}
			AutoMalloc<wchar_t> buffer(cchConversion);
			if (!MultiByteToWideChar(CP_ACP, 0, source, static_cast<int>(cchLength), buffer, static_cast<int>(cchConversion)))
			{
				auto error = GetLastError();
				throw CWinAPIErrorException(__FUNCTION__, "MultiByteToWideChar", error);
			}
			return std::move(buffer);
		}

	}
}
