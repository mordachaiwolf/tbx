#include "StdAfx.h"
#include "AutoHandle.h"
#include "bom.h"

#if _AFX // MFC only
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#endif

namespace Toolbox {

	size_t GetLength(BOM bom)
	{
		switch (bom)
		{
		case BOM::none:
			return 0;
		case BOM::utf8:
			return sizeof(kUTF8);
		case BOM::utf16le:
			return sizeof(kUTF16LE);
		case BOM::utf16be:
			return sizeof(kUTF16BE);
		case BOM::utf32le:
			return sizeof(kUTF32LE);
		case BOM::utf32be:
			return sizeof(kUTF32BE);
		default:
			throw std::invalid_argument(__FUNCTION__ " : Invalid BOM");
		}
	}

	void GetBOM(BOM bom, const BYTE * & pBytes, size_t & cbBytes)
	{
		switch (bom)
		{
		case BOM::utf8:
			pBytes = kUTF8;
			cbBytes = sizeof(kUTF8);
			break;
		case BOM::utf16le:
			pBytes = kUTF16LE;
			cbBytes = sizeof(kUTF16LE);
			break;
		case BOM::utf16be:
			pBytes = kUTF16BE;
			cbBytes = sizeof(kUTF16BE);
			break;
		case BOM::utf32le:
			pBytes = kUTF32LE;
			cbBytes = sizeof(kUTF32LE);
			break;
		case BOM::utf32be:
			pBytes = kUTF32BE;
			cbBytes = sizeof(kUTF32BE);
			break;
		default:
			throw std::invalid_argument(__FUNCTION__ " : Invalid Byte Order Marker");
		}
	}

	// diagnostic message (English verbose name)
	const TCHAR * GetName(BOM bom)
	{
		switch (bom)
		{
		case BOM::none:
			return _T("No byte ordering marker found");
		case BOM::utf8:
			return _T("UTF 8");
		case BOM::utf16le:
			return _T("UTF 16 little endian");
		case BOM::utf16be:
			return _T("UTF 16 big endian");
		case BOM::utf32le:
			return _T("UTF 32 little endian");
		case BOM::utf32be:
			return _T("UTF 32 big endian");
		case BOM::invalid:
			return _T("Invalid BOM");
		default:
			throw CLabeledException(_T("Invalid BOM!"));
		}
	}

	//////////////////////////////////////////////////////////////////////////

	template <size_t size>
	bool equal(const BYTE(&bom)[size], const BYTE * buffer, size_t cbBuffer)
	{
		return cbBuffer >= size && STRCMP_EQUAL(memcmp(bom, buffer, size));
	}

	BOM HasBOM(const BYTE * buffer, size_t cbBuffer)
	{
		if (equal(kUTF32LE, buffer, cbBuffer))
			return BOM::utf32le;
		if (equal(kUTF16LE, buffer, cbBuffer))
			return BOM::utf16le;
		if (equal(kUTF32BE, buffer, cbBuffer))
			return BOM::utf32be;
		if (equal(kUTF16BE, buffer, cbBuffer))
			return BOM::utf16be;
		if (equal(kUTF8, buffer, cbBuffer))
			return BOM::utf8;
		return BOM::none;
	}

	//////////////////////////////////////////////////////////////////////////

	bool CreateTextFile(const TCHAR * filename, bool bOverwriteExisting, BOM bom)
	{
		// open || create
		Toolbox::AutoHandle::AutoCloseFile hFile(::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, nullptr, bOverwriteExisting ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr));
		if (!hFile)
			return false;

		// get bytes to write
		const BYTE * bytes;
		size_t length;
		GetBOM(bom, bytes, length);

		// special case: zero length bom
		if (!length)
			return true;

		// write the bom & return status
		DWORD written;
		return ::WriteFile(hFile, bytes, length, &written, nullptr) && written == length;
	}

	bool ReadBOM(const TCHAR * filename, BOM & bom)
	{
		// open (must exist)
		Toolbox::AutoHandle::AutoCloseFile hFile(::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
		if (!hFile)
			return false;

		// read enough to determine even the largest BOM (32 bits / 4 bytes)
		BYTE buffer[4];
		DWORD cbRead;
		if (!::ReadFile(hFile, &buffer, sizeof(buffer), &cbRead, nullptr))
			return false;

		// determine what bom was there
		// NOTE: ReadFile may have read less than that requested, but that's okay here
		//       HasBOM will will work with whatever was returned
		bom = HasBOM(buffer, cbRead);

		// done
		return true;
	}

}
