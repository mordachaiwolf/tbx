#include "stdafx.h"

#include <shlobj.h>

#include "ApplicationInstanceRefCount.h"

namespace tbx {

	namespace Shell {

		ApplicationInstanceRefCount::ApplicationInstanceRefCount()
			: m_cRef(1)
			, m_dwThread(GetCurrentThreadId())
		{
			SHSetInstanceExplorer(this);
		}

		ApplicationInstanceRefCount::~ApplicationInstanceRefCount()
		{
			SHSetInstanceExplorer(nullptr);
			Release();

			MSG msg;
			while (m_cRef && GetMessage(&msg, nullptr, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		STDMETHODIMP ApplicationInstanceRefCount::QueryInterface(REFIID riid, void **ppv)
		{
			if (riid == IID_IUnknown)
			{
				*ppv = static_cast<IUnknown*>(this);
				AddRef();
				return S_OK;
			}

			*ppv = nullptr;
			return E_NOINTERFACE;
		}

		STDMETHODIMP_(ULONG) ApplicationInstanceRefCount::AddRef()
		{
			return InterlockedIncrement(&m_cRef);
		}

		STDMETHODIMP_(ULONG) ApplicationInstanceRefCount::Release()
		{
			LONG lRef = InterlockedDecrement(&m_cRef);
			if (lRef == 0)
				PostThreadMessage(m_dwThread, WM_NULL, 0, 0);
			return lRef;
		}

	}

}