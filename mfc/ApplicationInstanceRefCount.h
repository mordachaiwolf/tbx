#pragma once

namespace tbx {

	namespace Shell {

		// Formerly 'ProcessReference'
		//	from http://blogs.msdn.com/b/oldnewthing/archive/2008/05/28/8555658.aspx?wa=wsignin1.0
		//
		// Usage: Create a stack-instance in your main()
		//
		// ApplicationInstanceRefCount will keep your exe from exiting (the main thread)
		// while any shell extension running in our process exists (e.g. a property page for a file)
		//
		class ApplicationInstanceRefCount : public IUnknown
		{
		public:
			ApplicationInstanceRefCount();
			~ApplicationInstanceRefCount();

			STDMETHODIMP QueryInterface(REFIID riid, void **ppv) override;

			STDMETHODIMP_(ULONG) AddRef() override;
			STDMETHODIMP_(ULONG) Release() override;

		private:
			LONG m_cRef;
			DWORD m_dwThread;
		};

	}
}