#include "StdAfx.h"

#include "SimpleDropTarget.h"

#include <ShlObj_core.h>

#if _AFX // MFC only
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif
#endif

namespace tbx {

	namespace Shell {

		STDMETHODIMP SimpleDropTarget::QueryInterface(REFIID riid, void **ppv)
		{
			if (riid == IID_IUnknown || riid == IID_IDropTarget)
			{
				*ppv = static_cast<IUnknown*>(this);
				AddRef();
				return S_OK;
			}
			*ppv = nullptr;
			return E_NOINTERFACE;
		}

		STDMETHODIMP_(ULONG) SimpleDropTarget::AddRef()
		{
			return InterlockedIncrement(&m_cRef);
		}

		STDMETHODIMP_(ULONG) SimpleDropTarget::Release()
		{
			LONG cRef = InterlockedDecrement(&m_cRef);
			if (cRef == 0) delete this;
			return cRef;
		}

		STDMETHODIMP SimpleDropTarget::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
		{
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

		STDMETHODIMP SimpleDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
		{
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

		STDMETHODIMP SimpleDropTarget::DragLeave()
		{
			return S_OK;
		}

		STDMETHODIMP SimpleDropTarget::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
		{
			OpenFilesFrom(pdto);
			*pdwEffect &= DROPEFFECT_COPY;
			return S_OK;
		}

		SimpleDropTarget::SimpleDropTarget() : m_cRef(1)
		{
			// we're ensuring that the current "Explorer (shell) Host Application" not exit until we're done
			// IFF your application uses ApplicationInstanceRefCount (or similar) then you'll be sure not to exit before we're done
			SHGetInstanceExplorer(&m_pExplorerInstance);
		}

	} // Shell

} // tbx

