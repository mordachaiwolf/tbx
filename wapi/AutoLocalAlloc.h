#pragma once

namespace Toolbox {

/*
//////////////////////////////////////////////////////////////////////
// class AutoLocalAlloc
//
//	This class acts similarly to a std::unique_ptr<t>, but is instead 
//	used for allocation/deallocation of arbitrary memory chunks,
//	not objects of a fixed size.
//
//	It uses LocalAlloc/LocalFree APIs to manage the allocated memory.

template <typename T>
class AutoLocalAlloc  
{
public:
	explicit AutoLocalAlloc(size_t size = sizeof(T), int zero = 0) : 
		m_ptr((T*)::LocalAlloc(GMEM_FIXED, size)) 
	{
		if (!m_ptr)
			throw std::bad_alloc();
		::memset(m_ptr, zero, size);
	}

	explicit AutoLocalAlloc(T * p) :
		m_ptr(p)
	{
	}

	AutoLocalAlloc(AutoLocalAlloc & rhs) :
		m_ptr(rhs.release())
	{
	}

	AutoLocalAlloc & operator = (AutoLocalAlloc & rhs)
	{
		if (this != &rhs)
		{
			if (m_ptr)
				free();
			m_ptr = rhs.release();
		}
		return *this;
	}

	~AutoLocalAlloc()
	{
		if (m_ptr)
			free();
	}

	T * get() const
	{
		return m_ptr;
	}

	T * release()
	{
		T * p = m_ptr;
		m_ptr = nullptr;
		return p;
	}

	T & operator * () const
	{
		return *get();
	}

	T * operator -> () const
	{
		return get();
	}

	void free()
	{
		::LocalFree((HGLOBAL)m_ptr);
		m_ptr = nullptr;
	}

private:

	T * m_ptr;		// pointer to allocated memory
};

// AutoLocalHandle is much like LocalAlloc
// however, the fundamental element should be a handle-like type (q.v. DECLARE_HANDLE())
template <typename HandleType>
class AutoLocalHandle  
{
public:
	explicit AutoLocalHandle(HandleType handle) :
		m_handle(handle)
	{
		ASSERT(IsValid());
	}

	explicit AutoLocalHandle(DWORD cbSize = sizeof(HandleType)) :
		m_handle((HandleType)::LocalAlloc(LPTR, cbSize))
	{
	}

	AutoLocalHandle(AutoLocalHandle & rhs) :
		m_handle(rhs.Release())
	{
	}

	~AutoLocalHandle()
	{
		if (m_handle)
			free();
	}

	AutoLocalHandle & operator = (AutoLocalHandle & rhs)
	{
		if (this != &rhs)
		{
			if (m_handle)
				free();
			m_handle = rhs.Release();
		}
		return *this;
	}

// accessors

	HandleType get() const { return m_handle; }

// automatic conversion

	operator HandleType () const { return get(); }

// actions

	// release ownership of the HGLOBAL
	HandleType release()
	{
		HandleType handle = m_handle;
		m_handle = nullptr;
		return handle;
	}

	// free our global handle
	void free()
	{
		ASSERT(IsValid());

		// free our handle if the lock count is zero
		// NOTE: this is always true for GMEM_FIXED
		//ASSERT((::LocalFlags(m_handle) & GMEM_LOCKCOUNT) == 0);

		::LocalFree((HGLOBAL)m_handle);

		m_handle = nullptr;
	}

// debugging

	bool IsValid() const { return ::LocalFlags((HGLOBAL)m_handle) != GMEM_INVALID_HANDLE; }

private:
	HandleType	m_handle;
};
*/

} // namespace Toolbox
