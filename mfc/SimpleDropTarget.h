#pragma once

#include <atlcomcli.h>

namespace tbx {

	namespace Shell {

		// SimpleDropTarget is from The Old New Thing: 
		//	http://blogs.msdn.com/b/oldnewthing/archive/2010/05/03/10006065.aspx?wa=wsignin1.0
		// Simply create your own subclass that implements OpenFiles()
		// NOTE: the application must exist as long as we do!  Don't exit the application while an instance of this object (or any other hosted shell object) exists!!!
		class SimpleDropTarget : public IDropTarget
		{
		public:
			SimpleDropTarget();

			// *** IUnknown ***
			STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override;

			STDMETHODIMP_(ULONG) AddRef() override;

			STDMETHODIMP_(ULONG) Release() override;

			// *** IDropTarget ***
			STDMETHODIMP DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;

			STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;

			STDMETHODIMP DragLeave() override;

			STDMETHODIMP Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect) override;

		protected:
			virtual void OpenFilesFrom(IDataObject *pdto) = 0;		// only method that a subclass really needs to bother with

		private:
			LONG				m_cRef;
			CComPtr<IUnknown>	m_pExplorerInstance;
		};


	} // Shell

} // tbx

