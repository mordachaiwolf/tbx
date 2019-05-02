#pragma once

#include "noncopyable.h"

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// BufferedAdaptor<T>
	//
	//	Acts as a buffered proxy using pointer or reference semantics.
	//  apply() flushes the buffered value to the actual entity.
	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	class BufferedAdaptor
	{
	public:
		BufferedAdaptor<T>(T & refData)
			: m_buffer(refData)
			, m_data(refData)
		{
		}

		// allow move semantics
		BufferedAdaptor<T>(BufferedAdaptor<T> && rhs) = default;
		BufferedAdaptor<T> & operator = (BufferedAdaptor<T> && rhs) = default;

		// but disallow copy
		BufferedAdaptor<T>(const BufferedAdaptor<T> & rhs) = delete;
		BufferedAdaptor<T> & operator = (const BufferedAdaptor<T> & rhs) = delete;

		// accessors to the buffered data point
		T & operator * ()
		{
			return m_buffer;
		}

		T & operator -> ()
		{
			return m_buffer;
		}

		// auto conversion to the buffered data point
		operator T & ()
		{
			return m_buffer;
		}

		operator T * ()
		{
			return &m_buffer;
		}

		// allow direct assignment of a new value (to our buffered value holder)
		template <typename U> BufferedAdaptor<T> & operator = (const U & newValue)
		{
			m_buffer = newValue;
			return *this;
		}

		// apply the changes to the actual data point
		void apply()
		{
			m_data = m_buffer;
		}

	private:
		T		m_buffer;		// local buffered copy (most I/O refers to this member, so put it first)
		T &		m_data;			// reference to real data storage
	};

	// syntax helper (didn't we get the ability in C++20 to skip this BS?)
	template <typename T> auto make_buffered_adaptor(T & value) { return std::move(BufferedAdaptor<T>(value)); }

} // namespace
