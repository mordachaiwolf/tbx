#pragma once

#include "CustomException.h"

// we manage a string which acts as a raw buffer and is moved into the caller's target string on exit
// this should be about as efficient as it can be, assuming that the underlying string class supports move semantics

namespace tbx {

	template <typename string_t = std::string>
	class AutoStrBuffer
	{
		using string_type = string_t;
		using char_type = typename string_t::value_type;

		string_t *	m_string;
		size_t		m_length;
		string_t	m_buffer;

	public:

		// create a string buffer with enough space to handle length + null terminator
		AutoStrBuffer(string_t & str, size_t length, char_type fill = '\0') 
			: m_string(&str)
			, m_length(length)
			, m_buffer(length + 1, fill)
		{
			TBX_PRECONDITION(length < std::numeric_limits<size_t>::max());
		}

		~AutoStrBuffer()
		{
			if (HasOwnership())
			{
				// clean up the buffer
				m_buffer[m_length] = 0;
				m_buffer.resize(std::strlen(m_buffer.data()));

				// move it to the caller's string
				*m_string = std::move(m_buffer);
			}
		}

	// buffer length

		// returns buffer length (not including space for a null terminator)
		size_t length() const { return m_length; }

		// returns the real size (in characters) of our allocated buffer (including the space for the null terminator)
		size_t size() const { return m_length + 1; }

		// resize our buffer
		void resize(size_t newmaxlength) {
			TBX_PRECONDITION(HasOwnership());
			m_buffer.resize(newmaxlength);
			m_length = newmaxlength;
		}

	// pointer semantics

		char_type * get() { return m_buffer.data(); }
		operator char_type * () { return get(); }

		const char_type * get() const { return m_buffer.c_str(); }
		operator const char_type * () const { return get(); }

		// move

		AutoStrBuffer(AutoStrBuffer && rhs)
			: m_string(rhs.m_string)
			, m_length(rhs.m_length)
			, m_buffer(rhs.RelinquishBuffer())
		{
		}

		AutoStrBuffer & operator = (AutoStrBuffer && rhs)
		{
			if (this != &rhs)
			{
				// move contents to us
				m_string = rhs.m_string;
				m_length = rhs.m_length;
				m_buffer = rhs.RelinquishBuffer();
			}
			return *this;
		}

		bool HasOwnership() const { return m_string != nullptr; }

	private:

		string_t && RelinquishBuffer()
		{
			// mark our data as dead
			m_string = nullptr;
			m_length = 0;
			return std::move(m_buffer);
		}
	};
}