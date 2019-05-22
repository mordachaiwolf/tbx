#include "StdAfx.h"

#include "AcceleratorTable.h"
#include "tbx/strings.h"

//#include <fmt/format.h>
#include "fmt/include/fmt/format.h"

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
			throw CWinAPIErrorException(__FUNCTION__, "LoadAccelerators", fmt::format("%X,%X", (UINT_PTR)hInstance, nResourceID), error);
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
			throw CWinAPIErrorException(__FUNCTION__, "LoadAccelerators", fmt::format("%X,%s", (UINT_PTR)hInstance, narrow(pszResource)), error);
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
			throw CWinAPIErrorException(__FUNCTION__, "CreateAcceleratorTable", fmt::format("%p,%u", (void*)(pAccels), count), error);
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
					throw CWinAPIErrorException(__FUNCTION__, "DestroyAcceleratorTable", fmt::format("%X", (UINT_PTR)m_hAccel), error);
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
			m_accels.resize(count);
			const auto copies = ::CopyAcceleratorTable(m_hAccel, &m_accels[0], count);

			if (count != copies)
				throw CContextException(__FUNCTION__, fmt::format("CopyAcceleratorTable() only copied %d out of %d entries", copies, count));
		}
		return m_accels;
	}

}