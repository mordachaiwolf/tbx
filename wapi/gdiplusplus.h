#pragma once

#include "tbx/CustomException.h"

// link library
#pragma comment(lib, "gdiplus.lib")

namespace tbx::wapi {

	// returns a simple literal for the GDI+ status code
	std::string GetGDIPlusErrorMessage(Gdiplus::Status status);

	// returns GDI+ status and underlying Win32 error explanation if applicable
	// Warning: GetLastError() needs to be captured as close to the originating error as possible
	//			as it can be overwritten by subsequent unrelated errors
	std::string GetGDIPlusErrorMessage(Gdiplus::Status status, const DWORD error);

	class CGDIPlusException : public CContextException
	{
		Gdiplus::Status	status;
		DWORD			error;
	public:
		// api
		CGDIPlusException(const char * source, Gdiplus::Status status, DWORD error = GetLastError()) : CContextException(source, GetGDIPlusErrorMessage(status, error)), status(status), error(error) {}
		CGDIPlusException(const wchar_t * source, Gdiplus::Status status, DWORD error = GetLastError()) : CContextException(source, GetGDIPlusErrorMessage(status, error)), status(status), error(error) {}

		Gdiplus::Status GetStatus() const { return status; }
		DWORD GetError() const { return error; }
	};

	inline void SucceedOrThrow(const char * source, Gdiplus::Status status)
	{
		if (status != Gdiplus::Ok)
			throw CGDIPlusException(source, status);
	}

	class GdiplusSession
	{
	public:
		GdiplusSession();
		GdiplusSession(const Gdiplus::GdiplusStartupInput * gdiplusStartupInput, Gdiplus::GdiplusStartupOutput * gdiplusStartupOutput = nullptr);

		~GdiplusSession();

	private:
		ULONG_PTR	m_gdiplusToken;
	};

	inline Gdiplus::Size SizeFromSize(const SIZE & size) { return Gdiplus::Size(size.cx, size.cy); }

	// TODO: we need a non-ATL replacement for CComPtr
// 	Gdiplus::Image * CreateImageFromRawMemory(BYTE * pImageData, size_t cbSize)

	std::unique_ptr<Gdiplus::Bitmap> CreateScaledCopy(Gdiplus::Bitmap * pSource, const Gdiplus::Size & newSize);

}