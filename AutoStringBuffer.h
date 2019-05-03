#pragma once
#include <string>

// we manage a string which acts as a raw buffer and is moved into the caller's target string on exit
// this should be about as efficient as it can be, assuming that the underlying string class supports move semantics

namespace tbx {

	template <typename string_t = std::string>
	class AutoStrBuffer
	{
		using CharType = typename string_t::traits_type::char_type;

		string_t *	m_string;
		size_t		m_length;
		string_t	m_buffer;

	public:

		// create a string buffer with enough space to handle length + null terminator
		AutoStrBuffer(string_t & str, size_t length, CharType fill = '\0') 
			: m_string(&str)
			, m_length(length)
			, m_buffer(length + 1, fill)
		{
			assert(length < std::numeric_limits<size_t>::max());
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

	// pointer semantics

		CharType * get() { return m_buffer.data(); }
		operator CharType * () { return get(); }

		const CharType * get() const { return m_buffer.c_str(); }
		operator const CharType * () const { return get(); }

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