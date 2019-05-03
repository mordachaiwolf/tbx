#pragma once

#include <vector>

#include "WinAPIError.h"

namespace tbx::wapi {

	//////////////////////////////////////////////////////////////////////
	// AcceleratorTable
	//////////////////////////////////////////////////////////////////////

	class AcceleratorTable
	{
	public:
		// disallow copy + assignment
		//   We'd need to either do ref counting, using PIMPL idiom, or
		//   we'd need to implement unique_ptr style single-owner semantics
		// NOTE: ref counted PIMPL idiom would work well here - we internally do the
		//       release before load/create, which is 1/2 way there...
		//   However, the reality is that you can point to an instance of this and manage its lifetime yourself
		//   without the need for this class to support that explicitly
		AcceleratorTable(AcceleratorTable & rhs) = delete;
		AcceleratorTable & operator = (AcceleratorTable & rhs) = delete;

		using accels_type = std::vector<ACCEL>;
		using const_iterator = accels_type::const_iterator;

		// construction

		AcceleratorTable() :
			m_hAccel(nullptr),
			m_bCreated(false)
		{
		};

		explicit AcceleratorTable(UINT nResourceID, HINSTANCE hInstance) :
			m_hAccel(nullptr),
			m_bCreated(false)
		{
			Load(nResourceID, hInstance);
		};

		explicit AcceleratorTable(const TCHAR * pszResource, HINSTANCE hInstance) :
			m_hAccel(nullptr),
			m_bCreated(false)
		{
			Load(pszResource, hInstance);
		};

		AcceleratorTable(const ACCEL * pAccels, size_t nAccels) :
			m_hAccel(nullptr),
			m_bCreated(false)
		{
			Create(pAccels, nAccels);
		};

		virtual ~AcceleratorTable();

		// actions

		void Load(UINT nResourceID, HINSTANCE hInstance);
		void Load(const TCHAR * pszResource, HINSTANCE hInstance);
		void Create(const ACCEL * pAccels, size_t count);
		void Clear() { Release(); }

		// attempt to dispatch this message for the given window (q.v. TranslateAccelerator)
		bool Dispatch(HWND hWnd, MSG * pMsg) const
		{
			return m_hAccel && ::TranslateAccelerator(hWnd, m_hAccel, pMsg) != 0;
		}

		// table accessors

		HACCEL GetHandle() const { return m_hAccel; }
		operator HACCEL () const { return GetHandle(); }

		// individual accessors

		size_t size() const { return GetAccels().size(); }

		const ACCEL & operator [] (int i) const { return GetAccels()[i]; }
		const const_iterator begin() const { return GetAccels().begin(); }
		const const_iterator end() const { return GetAccels().end(); }

	protected:

		void Release();

		accels_type & GetAccels() const;

		HACCEL					m_hAccel;		// the handle to our accel table
		bool					m_bCreated;		// true if we called CreateAcceleratorTable
		mutable accels_type		m_accels;		// cached copy of the accel table
	};


	// simple binder to glue a AcceleratorTable to a window
	class AcceleratorBinder
	{
	public:

		// Construction

		AcceleratorBinder(HWND wnd, AcceleratorTable & accel_table) :
			m_wnd(wnd), m_accel_table(accel_table)
		{
		};

		// Actions

		bool Dispatch(MSG * pMsg) const
		{
			TBX_PRECONDITION(m_wnd);
			TBX_PRECONDITION(pMsg);
			return m_accel_table.Dispatch(m_wnd, pMsg);
		}

	private:
		HWND				m_wnd;
		AcceleratorTable &	m_accel_table;
	};

}
