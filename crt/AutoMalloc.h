#pragma once

#include <assert.h>

namespace tbx
{

	//////////////////////////////////////////////////////////////////////
	// AutoMalloc
	//	This class acts similarly to a std::unique_ptr<t>, but is instead
	//	 used for allocation/deallocation of arbitrary arrays of memory,
	//	 not a single object.
	//	As the name implies, it uses malloc/free to manage the allocated memory
	//  This class uses C-array semantics, and can be used as a replacement for
	//   most such consumers (not transparently, however).
	//////////////////////////////////////////////////////////////////////

	template <typename T = unsigned char>
	class AutoMalloc
	{
	public:

		// NOTE:
		//  For auto pointer types:
		//	  One can either have a ctor that takes the raw pointer type
		//	  OR automatic conversion operators
		//	  BUT NEVER BOTH!!!
		//
		//	  If you try to have them both, then C++ uses the conversion operators to feed the raw pointer ctor,
		//	  effectively bypassing your copy ctor & leading to a instance of your smart pointer referring to a dead
		//    memory location instead of a valid entity.
		//
		//	  For our purposes, we disallow construction from a raw pointer, but allow automatic conversion
		//    to our underlying pointer type.
		//
		//	explicit AutoMalloc(T * p) :
		//		m_ptr(p)
		//	{
		//	}

		using Elem = T;

		// allocate a block of given size (in bytes)
		explicit AutoMalloc<T>(size_t count = 0) :
			m_cbSize(count * sizeof(T)),
			m_ptr(count ? (T*)::malloc(m_cbSize) : nullptr)
		{
			if (!m_ptr && count)
				throw std::bad_alloc();
		}

		// allocate a block of given size, and copy the specified pointer's contents into ours
		AutoMalloc<T>(const T * pMemory, size_t count) :
			m_cbSize(count * sizeof(T)),
			m_ptr(count ? (T*)::malloc(m_cbSize) : nullptr)
		{
			if (m_ptr)
				::memcpy(m_ptr, pMemory, m_cbSize);
		}

		~AutoMalloc<T>()
		{
			if (m_ptr)
				free();
		}

		// copy semantics (non-copyable)
 		AutoMalloc<T>(const AutoMalloc<T> &) = delete;
 		AutoMalloc<T> & operator = (const AutoMalloc<T> &) = delete;

		// move semantics
		AutoMalloc<T>(AutoMalloc<T> && rhs) :
			m_cbSize(rhs.size_in_bytes()),
			m_ptr(rhs.release())
		{
		}

		AutoMalloc<T> & operator = (AutoMalloc<T> && rhs) { return take(std::move(rhs)); }

		AutoMalloc<T> & take(AutoMalloc<T> && that)
		{
			if (this != &that)
			{
				if (m_ptr)
					free();
				m_cbSize = that.m_cbSize;
				m_ptr = that.release();
			}
			return *this;
		}

		// take another buffer as our own (this is a casting operation!)
		template<typename U>
		AutoMalloc<T> & take_cast(AutoMalloc<U> && that)
		{
			if (that.size_in_bytes() % sizeof(T) != 0)
				throw std::invalid_argument(__FUNCTION__ " : invalid conversion!");
			return take((AutoMalloc<T>&&)that);
		}

		size_t size() const
		{
			return m_cbSize / sizeof(T);
		}

		size_t size_in_bytes() const
		{
			return m_cbSize;
		}

// explicit buffer access

		T * get()
		{
			return m_ptr;
		}

		const T * get() const
		{
			return m_ptr;
		}

// implicit buffer access (auto conversions)

		operator T * ()
		{
			return m_ptr;
		}

		operator const T * () const
		{
			return m_ptr;
		}

		// release our buffer to caller (we relinquish ownership & responsibility for it)
		T * release()
		{
			T * p = m_ptr;
			m_cbSize = 0;
			m_ptr = nullptr;
			return p;
		}

		// deallocates us
		void free()
		{
			if (m_ptr)
			{
				::free(m_ptr);
				m_ptr = nullptr;
				m_cbSize = 0;
			}
			assert(!m_cbSize);
			assert(!m_ptr);
		}

		// synonym for free to make us more interchangeable with std smart pointers
		void reset() { free(); }

		// erases our contents
		void erase(int zero_value = 0)
		{
			::memset(m_ptr, zero_value, m_cbSize);
		}

		// reallocate us 
		//  zero count is equivalent to calling free()
		//	failure to allocate new size is also equivalent to free()
		AutoMalloc & realloc(size_t count)
		{
			if (count)
			{
				m_cbSize = count * sizeof(T);
				auto * new_ptr = (T*)::realloc(m_ptr, m_cbSize);
				if (new_ptr)
					m_ptr = new_ptr;
				else
					free();
			}
			else
				free();
			return *this;
		}

		// returns a new copy of ourselves
		AutoMalloc<T> copy() const
		{
			return AutoMalloc<T>(m_ptr, m_cbSize);
		}

		// copies the given memory into ourself (and resizes us accordingly)
		template <typename X>
		AutoMalloc<T> & copy(const X * ptr, size_t count_of_x)
		{
			realloc(MulDiv(count_of_x, sizeof(X), sizeof(T)));
			::memmove(m_ptr, ptr, m_cbSize);
			return *this;
		}

		// swap our contents
		void swap(AutoMalloc<T> & rhs)
		{
			// if swap<size_t> and swap<T*> cannot throw, then neither can we
			std::swap(m_cbSize, rhs.m_cbSize);
			std::swap(m_ptr, rhs.m_ptr);
		}

	private:
		size_t	m_cbSize;		// size of allocated block (in bytes)
		T *		m_ptr;			// pointer to allocated memory
	};

	template <typename T>
	inline void free(AutoMalloc<T> & p)
	{
		p.free();
	}

	template <typename T>
	inline void swap(AutoMalloc<T> & lhs, AutoMalloc<T> & rhs)
	{
		lhs.swap(rhs);
	}

	// ensure that Zero(AutoMalloc<T>&) resolves properly
	template <typename T>
	inline void Zero(AutoMalloc<T> & p, int zero_value = 0)
	{
		p.erase(zero_value);
	}

} // namespace tbx
