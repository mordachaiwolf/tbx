#include "stdafx.h"

#include "gdiplusplus.h"

#include "tbx/CustomException.h"
#include "WinAPIError.h"

namespace tbx::wapi {

	std::string GetGDIPlusErrorMessage(Gdiplus::Status status)
	{
		using namespace Gdiplus;
		switch (status)
		{
		case Ok:
			return "Gdiplus::Ok";
		case GenericError:
			return "Gdiplus::GenericError";
		case InvalidParameter:
			return "Gdiplus::InvalidParameter";
		case OutOfMemory:
			return "Gdiplus::OutOfMemory";
		case ObjectBusy:
			return "Gdiplus::ObjectBusy";
		case InsufficientBuffer:
			return "Gdiplus::InsufficientBuffer";
		case NotImplemented:
			return "Gdiplus::NotImplemented";
		case Win32Error:
			return "Gdiplus::Win32Error";
		case WrongState:
			return "Gdiplus::WrongState";
		case Aborted:
			return "Gdiplus::Aborted";
		case FileNotFound:
			return "Gdiplus::FileNotFound";
		case ValueOverflow:
			return "Gdiplus::ValueOverflow";
		case AccessDenied:
			return "Gdiplus::AccessDenied";
		case UnknownImageFormat:
			return "Gdiplus::UnknownImageFormat";
		case FontFamilyNotFound:
			return "Gdiplus::FontFamilyNotFound";
		case FontStyleNotFound:
			return "Gdiplus::FontStyleNotFound";
		case NotTrueTypeFont:
			return "Gdiplus::NotTrueTypeFont";
		case UnsupportedGdiplusVersion:
			return "Gdiplus::UnsupportedGdiplusVersion";
		case GdiplusNotInitialized:
			return "Gdiplus::GdiplusNotInitialized";
		case PropertyNotFound:
			return "Gdiplus::PropertyNotFound";
		case PropertyNotSupported:
			return "Gdiplus::PropertyNotSupported";
		default:
			return "Gdiplus::UnknownStatusCode";
		}
	}

	std::string GetGDIPlusErrorMessage(Gdiplus::Status status, const DWORD error)
	{
		if (status == Gdiplus::Win32Error)
			return GetGDIPlusErrorMessage(status) + " - " + GetWinAPIErrorMessage(error);
		else
			return GetGDIPlusErrorMessage(status);
	}

	GdiplusSession::GdiplusSession()
	{
		// Initialize GDI+
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		SucceedOrThrow(__FUNCTION__, GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, nullptr));
	}

	GdiplusSession::GdiplusSession(const Gdiplus::GdiplusStartupInput * gdiplusStartupInput, Gdiplus::GdiplusStartupOutput * gdiplusStartupOutput /*= nullptr*/)
	{
		// Initialize GDI+
		SucceedOrThrow(__FUNCTION__, GdiplusStartup(&m_gdiplusToken, gdiplusStartupInput, gdiplusStartupOutput));
	}

	GdiplusSession::~GdiplusSession()
	{
		// Shutdown GDI+
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
	}


// 	Gdiplus::Image * CreateImageFromRawMemory(BYTE * pImageData, size_t cbSize)
// 	{
// 		// create an IStream on the HGLOBAL image data
// 		ATL::CComPtr<IStream> stream;
// 		if (FAILED(CreateStreamOnRawMemory(pImageData, cbSize, &stream)))
// 			return nullptr;
// 
// 		// now we can create an image from the stream
// 		return Gdiplus::Image::FromStream(stream);
// 	}

	std::unique_ptr<Gdiplus::Bitmap> CreateScaledCopy(Gdiplus::Bitmap * pSource, const Gdiplus::Size & newSize)
	{
		// create a target bitmap
		auto bm = std::make_unique<Gdiplus::Bitmap>(newSize.Width, newSize.Height);
		SucceedOrThrow(__FUNCTION__, bm->GetLastStatus());

		// create a drawing context on buffer
		Gdiplus::Graphics gr(bm.get());
		SucceedOrThrow(__FUNCTION__, gr.GetLastStatus());

		// draw the scaled image to the buffered context
		gr.DrawImage(pSource, 0, 0, newSize.Width, newSize.Height);
		SucceedOrThrow(__FUNCTION__, gr.GetLastStatus());

		return bm;
	}

}