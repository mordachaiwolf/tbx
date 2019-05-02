#pragma once

// AutoHandleTemplate
//
// This code was inspired by: http://www.codeproject.com/cpp/AutoClose.asp
// And then enhanced with inspiration from: http://www.codeproject.com/cpp/template2003.asp
//
// AutoHandleTemplate is a simple wrapper for any handle that must be released/freed/closed/etc. after being loaded/allocated/opened/etc.
// Due to the somewhat random nature of the Windows API's rules about how a handle is to be closed after being opened,
// this class allows the programmer to select the appropriate pairing of open to close, alloc to free, etc.
// For example: an HICON either needs to be destroyed via DestroyIcon if it was generated through CreateIcon,
// or it can be ignored, since it will automatically be unloaded when the application terminates if it was generated
// through LoadIcon or another resource load based operation.
//
//

#include "noncopyable.h"

// we embed the auto handle stuff in its own sub-namespace to allow for using namespace Toolbox::AutoHandle;

namespace Toolbox {
	namespace AutoHandle {

		template<typename HandlePolicy>
		class AutoHandleTemplate
		{
		public:
			using handle_t = typename HandlePolicy::handle_t;
			using this_type = AutoHandleTemplate<HandlePolicy>;

			AutoHandleTemplate() : m_handle(HandlePolicy::InvalidHandleValue()) { }
			explicit AutoHandleTemplate(handle_t handle) : m_handle(handle) { }

			~AutoHandleTemplate()
			{
				DisposeAndReplaceWith(HandlePolicy::InvalidHandleValue());
			}

			AutoHandleTemplate(const this_type & rhs) : m_handle(HandlePolicy::InvalidHandleValue())
			{
				*this = const_cast<this_type &>(rhs);
			}

			this_type & operator = (this_type & rhs)
			{
				// force rhs to release the handle to us
				if (this != &rhs)
					this->operator = (rhs.Detach());

				return *this;
			}

			this_type & operator = (handle_t handle)
			{
				if (handle != m_handle)
					DisposeAndReplaceWith(handle);

				return *this;
			}

			handle_t Detach()
			{
				handle_t handle = m_handle;
				m_handle = HandlePolicy::InvalidHandleValue();
				return handle;
			}

			// implicit access
			operator handle_t () const { return m_handle; }

			// explicit access
 			handle_t get() const { return m_handle; }

			// write access (only allowed if we're currently null!)
			// this makes it easier to use us for functions like RegOpenKey...
			handle_t & expose_handle()
			{
				if (m_handle != HandlePolicy::InvalidHandleValue()) 
					throw CLabeledException(__FUNCTION__, _T("Invalid attempt to directly access the underlying handle for writing when we currently already have an open handle there!")); 
				return m_handle; 
			}

			// we explicitly define bool conversions because the compiler would otherwise happily use the handle_t conversion for if (instance) and if (!instance)
			// so by explicitly defining these, we don't use zero or non-zero, but invalid_handle or not.
			// we do NOT use undefined_bool technique here, because the operator handle_t makes that technique ambiguous (two integral operators)
			operator bool() const { return m_handle != HandlePolicy::InvalidHandleValue(); }
			bool operator ! () const { return m_handle == HandlePolicy::InvalidHandleValue(); }

			// close this handle (sets our handle the invalid handle value)
			void Close() { DisposeAndReplaceWith(); }

		private:
			handle_t m_handle;

			void DisposeAndReplaceWith(handle_t handle = HandlePolicy::InvalidHandleValue())
			{
				if (m_handle != HandlePolicy::InvalidHandleValue())
					HandlePolicy::Close(m_handle); //, MessageBeep(IDOK);

				m_handle = handle;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		// Useful type definitions

		struct DestroyIconHandlePolicy
		{
			using handle_t = HICON;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hIcon) { ::DestroyIcon(hIcon); }
		};

		struct GenericHandlePolicy
		{
			using handle_t = HANDLE;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hHandle) { ::CloseHandle(hHandle); }
		};

		struct FileHandlePolicy
		{
			using handle_t = HANDLE;
			static const handle_t InvalidHandleValue() { return INVALID_HANDLE_VALUE; }
			static void Close(handle_t hFile) { ::CloseHandle(hFile); }
		};

		struct ModuleHandlePolicy
		{
			using handle_t = HMODULE;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hModule) { ::FreeLibrary(hModule); }
		};

		struct RegKeyHandlePolicy
		{
			using handle_t = HKEY;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hKey) { ::RegCloseKey(hKey); }
		};

		template <typename HandleType>
		struct DeleteGdiObjPolicy
		{
			using handle_t = HandleType;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hObject) { ::DeleteObject(hObject); }
		};

		struct DeleteDCPolicy
		{
			using handle_t = HDC;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hDC) { ::DeleteDC(hDC); }
		};

		template <HWND hwnd>
		struct ReleaseDCPolicy
		{
			using handle_t = HDC;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hDC) { ::ReleaseDC(hwnd, hDC); }
		};

		struct DestroyCursorPolicy
		{
			using handle_t = HCURSOR;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hCursor) { ::DestroyCursor(hCursor); }
		};

		struct FreeGlobalPolicy
		{
			using handle_t = HGLOBAL;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t hGlobal) { ::GlobalFree(hGlobal); }
		};

		using AutoCloseHandle = AutoHandleTemplate<GenericHandlePolicy>;
		using AutoCloseFile = AutoHandleTemplate<FileHandlePolicy>;
		using AutoFreeLibrary = AutoHandleTemplate<ModuleHandlePolicy>;
		using AutoCloseRegKey = AutoHandleTemplate<RegKeyHandlePolicy>;
		using AutoDeleteGdiObj = AutoHandleTemplate<DeleteGdiObjPolicy<HGDIOBJ>>;
		using AutoDeleteImage = AutoDeleteGdiObj;
		using AutoDeleteFont = AutoHandleTemplate<DeleteGdiObjPolicy<HFONT>>;
		using AutoDeleteBitmap = AutoHandleTemplate<DeleteGdiObjPolicy<HBITMAP>>;
		using AutoDeleteBrush = AutoHandleTemplate<DeleteGdiObjPolicy<HBRUSH>>;
		using AutoDeleteRgn = AutoHandleTemplate<DeleteGdiObjPolicy<HRGN>>;
		using AutoDestroyIcon = AutoHandleTemplate<DestroyIconHandlePolicy>;	// note: this is only appropriate for a created icon, not a loaded one!
		using AutoDestroyCursor = AutoHandleTemplate<DestroyCursorPolicy>;	// note: this is only appropriate for a created cursor, not a loaded one!
		using AutoDeleteDC = AutoHandleTemplate<DeleteDCPolicy>;
		//struct AutoReleaseDC AutoHandleTemplate<ReleaseDCPolicy>; // Just use CClientDC instead!
		using AutoFreeGlobal = AutoHandleTemplate<FreeGlobalPolicy>;

		struct DeleteBSTRPolicy
		{
			using handle_t = BSTR;
			static const handle_t InvalidHandleValue() { return nullptr; }
			static void Close(handle_t sysstring) { ::SysFreeString(sysstring); }
		};

		using AutoBSTR = AutoHandleTemplate<DeleteBSTRPolicy>;


	} // AutoHandle
} // Toolbox
