#pragma once

#include <new>
#include "noncopyable.h"

namespace Toolbox {

	//////////////////////////////////////////////////////////////////////
	//
	// Designer's notes:
	//
	//	My intention is that user code should use one of the following classes:
	//
	//	AutoGlobalPtr
	//		gives simple access to allocating & manipulating a global block of memory using 
	//		pointer semantics.
	//
	//	AutoGlobalHandleFixed
	//		offers simple access to allocating & manipulating a global handle using handle
	//		semantics; also offers simple access & manipulation as a pointer
	//	
	//	AutoGlobalHandleMoveable
	//		offers simple access to allocating & manipulating a movable global memory handle
	//		this classes uses AutoGlobalLockedPtr to allow pointer access 
	//	
	//////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////
	// class AutoGlobalHandle
	//
	//	Wraps ::GlobalAlloc and ::GlobalFree.
	//	This class makes no assumptions about how the HGLOBAL was allocated
	//  Subclasses are provided for your convenience to allow pointer, fixed handle, and 
	//	movable handle style manipulation of global memory.
	//
	//	NOTE: LocalAlloc and GlobalAlloc DO THE SAME THING in Win32 programming,
	//        so it is possible to use a GlobalAlloc wherever a LocalAlloc
	//		  is referred to (however, under WinCE, this is not true).
	//////////////////////////////////////////////////////////////////////

	template <class HandleType = HGLOBAL>
	class AutoGlobalHandle
	{
	public:
		using handle_type = HandleType;		// alias for type of handle to use

	// construction

		AutoGlobalHandle(UINT flags, size_t size) :
			m_handle((HandleType)::GlobalAlloc(flags, size))
		{
			if (!IsValid())
				throw std::bad_alloc();		// q.v. #include <new>
		}

		explicit AutoGlobalHandle(HandleType handle) :
			m_handle(handle)
		{
		}

		AutoGlobalHandle(AutoGlobalHandle<HandleType> & rhs) :
			m_handle(rhs.release())
		{
		}

		~AutoGlobalHandle()
		{
			if (m_handle)
				free();
		}

		// assignment

		AutoGlobalHandle & operator = (AutoGlobalHandle<HandleType> & rhs)
		{
			if (this != &rhs)
			{
				if (m_handle)
					free();
				m_handle = rhs.release();
			}
			return *this;
		}

		// accessors

		HandleType get()
		{
			return m_handle;
		}

		// resource management

		HandleType release()
		{
			HandleType handle = m_handle;
			m_handle = nullptr;
			return handle;
		}

		void free()
		{
			ASSERT(IsValid());
			VERIFY(!::GlobalFree((HGLOBAL)m_handle));
			m_handle = nullptr;
		}

		// debugging

		bool IsValid() const
		{
			return ::GlobalFlags((HGLOBAL)m_handle) != GMEM_INVALID_HANDLE;
		}

	private:

		HandleType	m_handle;		// handle for allocated memory
	};


	// class AutoGlobalHandleFixed
	//
	//	This class acts similarly to a std::unique_ptr<t>, but is instead 
	//	used for allocation/deallocation of arbitrary memory chunks in the Win32 Global Heap.
	//	Additionally, memory can be sized at ctor time (not necessarily based on the 
	//	object type).

	template <class HandleType = HGLOBAL, class ObjType = BYTE>
	class AutoGlobalHandleFixed : public AutoGlobalHandle<HandleType>
	{
	public:
		// construction

		AutoGlobalHandleFixed(size_t size = sizeof(ObjType)) :
			AutoGlobalHandle<HandleType>(GMEM_FIXED, size)
		{
		}

		explicit AutoGlobalHandleFixed(HandleType handle) :
			AutoGlobalHandle<HandleType>(handle)
		{
			// NOTE: you *must* have allocated handle using GMEM_FIXED (or an equivalent!!!)
		}

		explicit AutoGlobalHandleFixed(ObjType * ptr) :
			AutoGlobalHandle<HandleType>((HandleType)ptr)
		{
			// NOTE: you *must* have allocated ptr using GMEM_FIXED (or an equivalent!!!)
		}

		// allow default copy ctor

		// allow default dtor

	// assignment

		// allow default assignment operator

	// accessors

		HandleType getHandle()
		{
			return AutoGlobalHandle<HandleType>::get();
		}

		ObjType * getPtr()
		{
			// the handle is equivalent to a pointer (q.v. ::GlobalAlloc(GMEM_FIXED))
			return (ObjType*)AutoGlobalHandle<HandleType>::get();
		}

		// automatic conversion

		operator HandleType ()
		{
			return getHandle();
		}

		operator ObjType * ()
		{
			return getPtr();
		}

		// pointer semantics

		ObjType & operator * ()
		{
			return *getPtr();
		}

		ObjType * operator -> ()
		{
			return getPtr();
		}
	};

	// class AutoGlobalPtr
	//
	//	This class acts similarly to a std::unique_ptr<t>, but is instead 
	//	used for allocation/deallocation of arbitrary memory chunks in the Win32 Global Heap.
	//	Additionally, memory can be sized at ctor time (not necessarily based on the 
	//	object type).
	template <class ObjType, class HandleType = HGLOBAL>
	class AutoGlobalPtr : public AutoGlobalHandleFixed<HandleType, ObjType>
	{
	public:
		// construction

		AutoGlobalPtr(size_t size = sizeof(ObjType)) :
			AutoGlobalHandleFixed<HandleType, ObjType>(size)
		{
		}

		explicit AutoGlobalPtr(HandleType handle) :
			AutoGlobalHandleFixed<HandleType>(handle)
		{
			// NOTE: you *must* have allocated handle using GMEM_FIXED (or an equivalent!!!)
		}

		explicit AutoGlobalPtr(ObjType * ptr) :
			AutoGlobalHandleFixed<HandleType>(ptr)
		{
			// NOTE: you *must* have allocated ptr using GMEM_FIXED (or an equivalent!!!)
		}

		// allow default copy ctor

		// allow default dtor

	// assignment

		// allow default assignment operator

	// accessors

		ObjType * get()
		{
			return getPtr();
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////
	// Movable Global Memory
	//////////////////////////////////////////////////////////////////////////////////////////

	// forward declaration
	template <class ObjType, class HandleType> class AutoGlobalLockedPtr;

	// AutoGlobalHandleMoveable is much like AutoGlobalHandle
	// however, the fundamental element should be a handle-like type (q.v. DECLARE_HANDLE())
	// and the memory is allocated using GMEM_MOVEABLE, which necessitates the use of
	// ::GlobalLock in order to obtain a pointer to the memory.
	// This is facilitated via the use of a AutoGlobalLockedPtr (q.v.)
	template <class ObjType, class HandleType = HGLOBAL>
	class AutoGlobalHandleMoveable : public AutoGlobalHandle<HandleType>
	{
	public:
		// construction

		AutoGlobalHandleMoveable(size_t size = sizeof(ObjType)) :
			AutoGlobalHandle<HandleType>(GMEM_MOVEABLE, size)
		{
		}

		explicit AutoGlobalHandleMoveable(HandleType * handle) :
			AutoGlobalHandle<HandleType>(handle)
		{
			// NOTE: you *must* have allocated handle using GMEM_MOVEABLE (or an equivalent!!!)
		}

		// allow default copy ctor

		// allow default dtor

	// assignment

		// allow default assignment operator

	// pointer access

		AutoGlobalLockedPtr<ObjType, HandleType> getPtr()
		{
			return AutoGlobalLockedPtr<ObjType, HandleType>(get());
		}
	};


	// AutoGlobalLockedPtr
	//  is used to temporarily get a pointer to a GMEM_MOVABLE HGLOBAL
	//  the lock is released when this instance is destroyed
	template <class ObjType, class HandleType = HGLOBAL>
	class AutoGlobalLockedPtr : protected noncopyable
	{
	public:
		using object_type = ObjType;		// alias for type of contents of our memory
		using handle_type = HandleType;		// alias for type of our handle

	// constructors

		explicit AutoGlobalLockedPtr(HandleType handle) :
			m_ptr((ObjType*)::GlobalLock((HGLOBAL)handle))
		{
			//ASSERT(GlobalHandle(m_ptr) != handle);
		}

		explicit AutoGlobalLockedPtr(AutoGlobalHandleMoveable<ObjType, HandleType> & rhs) :
			m_ptr((ObjType*)::GlobalLock((HGLOBAL)rhs.get()))
		{
		}

		// move ctor
		AutoGlobalLockedPtr(AutoGlobalLockedPtr<ObjType, HandleType> && rhs) :
			m_ptr(rhs.release())
		{
		}

		~AutoGlobalLockedPtr()
		{
			if (m_ptr)
				free();
		}

		// assignment

		AutoGlobalLockedPtr & operator = (const AutoGlobalLockedPtr<ObjType, HandleType> && rhs)
		{
			if (this != rhs)
				m_ptr = rhs.release();
			return *this;
		}

		// accessors

		ObjType * get()
		{
			return m_ptr;
		}

		const ObjType * get() const
		{
			return m_ptr;
		}

		HandleType getHandle()
		{
			return (HandleType)::GlobalHandle(m_ptr);
		}

		// automatic conversion

		operator ObjType * ()
		{
			return get();
		}

		operator const ObjType * () const
		{
			return get();
		}

		// pointer semantics

		ObjType & operator * ()
		{
			return *get();
		}

		const ObjType & operator * () const
		{
			return *get();
		}

		ObjType * operator -> ()
		{
			return get();
		}
		const ObjType * operator -> () const
		{
			return get();
		}

		// array access

		ObjType & operator [] (int index)
		{
			return *(get() + sizeof(T) * index);
		}

		const ObjType & operator [] (int index) const
		{
			return *(get() + sizeof(T) * index);
		}

		// resource management

			// release ownership of our global lock
		ObjType * release()
		{
			ObjType * ptr = m_ptr;
			m_ptr = nullptr;
			return ptr;
		}

		// free our lock
		void free()
		{
			if (m_ptr)
			{
				HGLOBAL handle = ::GlobalHandle(m_ptr);
				if (handle)
				{
					ASSERT(::GlobalFlags(handle) != GMEM_INVALID_HANDLE);
					VERIFY(::GlobalUnlock(handle) || ::GetLastError() == NO_ERROR);
				}
				m_ptr = nullptr;
			}
		}

		// debugging

		bool IsValid() const
		{
			if (m_ptr == nullptr)
				return true;
			HGLOBAL handle = ::GlobalHandle(m_ptr);
			if (!handle)
				return false;
			return ::GlobalFlags(handle) != GMEM_INVALID_HANDLE;
		}

	private:
		ObjType *	m_ptr;
	};

	// helper functions
	template <typename ObjType, typename HandleType>
	AutoGlobalLockedPtr<ObjType, HandleType>
		GetAutoGlobalPtr(AutoGlobalHandleMoveable<ObjType, HandleType> & auto_global)
	{
		return AutoGlobalLockedPtr<ObjType, HandleType>(auto_global);
	}


} // namespace Toolbox
