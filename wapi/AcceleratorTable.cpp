#include "StdAfx.h"

#include "AcceleratorTable.h"
#include "tbx/strings.h"

namespace tbx::wapi {

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////

	AcceleratorTable::~AcceleratorTable()
	{
		Release();
	}

	void AcceleratorTable::Load(UINT nResourceID, HINSTANCE hInstance)
	{
		Release();

		m_hAccel = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(nResourceID));
		if (!m_hAccel)
		{
			auto error = GetLastError();
			throw CWinAPIErrorException(__FUNCTION__, "LoadAccelerators", format_string("%X,%X", hInstance, nResourceID), error);
		}
	}

	void AcceleratorTable::Load(const TCHAR * pszResource, HINSTANCE hInstance)
	{
		TBX_PRECONDITION(pszResource != nullptr);

		Release();

		m_hAccel = ::LoadAccelerators(hInstance, pszResource);
		if (!m_hAccel)
		{
			auto error = GetLastError();
			throw CWinAPIErrorException(__FUNCTION__, "LoadAccelerators", format_string("%X,%s", hInstance, pszResource), error);
		}
	}

	void AcceleratorTable::Create(const ACCEL * pAccels, size_t count)
	{
		TBX_PRECONDITION(pAccels != nullptr);

		// first, release previous table
		Release();

		m_bCreated = true;
		m_hAccel = ::CreateAcceleratorTable(const_cast<LPACCEL>(pAccels), (int)count);
		if (!m_hAccel)
		{
			auto error = GetLastError();
			throw CWinAPIErrorException(__FUNCTION__, "CreateAcceleratorTable", format_string("%p,%d", pAccels, count), error);
		}
	}

	void AcceleratorTable::Release()
	{
		if (m_bCreated && m_hAccel)
		{
			if (!::DestroyAcceleratorTable(m_hAccel))
				if (!m_hAccel)
				{
					auto error = GetLastError();
					throw CWinAPIErrorException(__FUNCTION__, "DestroyAcceleratorTable", format_string("%X", m_hAccel), error);
				}
		}
		m_hAccel = nullptr;
		m_bCreated = false;
		m_accels.clear();
	}

	AcceleratorTable::accels_type & AcceleratorTable::GetAccels() const
	{
		if (m_hAccel && !m_accels.size())
		{
			const auto count = ::CopyAcceleratorTable(m_hAccel, nullptr, 0);
			m_accels = accels_type(count);
			const auto copies = ::CopyAcceleratorTable(m_hAccel, &m_accels[0], count);

			if (count != copies)
				throw CContextException(__FUNCTION__, format_string("CopyAcceleratorTable() only copied %d out of %d entries", copies, count));
		}
		return m_accels;
	}

}