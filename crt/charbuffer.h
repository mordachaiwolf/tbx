#pragma once

#include "SmartChar.h"

namespace Toolbox {

	//////////////////////////////////////////////////////////////////////////
	// charbuffer class
	//
	//	fixed-length character buffer
	//
	//	useful in situations where you're accumulating something into a
	//  character buffer of known maximum size
	//
	//  we treat lengths as count of base-character type for ANSI or UTF-16 strings
	//  (count of bytes or words, respectively)
	//////////////////////////////////////////////////////////////////////////

	template <typename CharType>
	class charbuffer : private StringPolicySelector<CharType>::StringPolicy
	{
	public:
		charbuffer(CharType * pszBuffer, size_t cchMaxLength)
			: m_buffer(pszBuffer)
			, m_capacity(cchMaxLength)
			, m_used(get_length(pszBuffer))
		{
			// our implementation requires non zero (see assertHasCapacityFor())
			ASSERT(m_capacity);
			AssertHasCapacityFor(1);
		}

		// automatic conversion to read only string
		operator const CharType * () const { return m_buffer; }

		// returns our current string length
		size_t length() const { return m_used; }

		// reset our buffer
		void erase() { m_used = 0; m_buffer[0] = 0; }

		// appends the given string (errors if we exceed our capacity)
		void append(const CharType * psz)
		{
			size_t len = get_length(psz);
			AssertHasCapacityFor(len);
			copy(m_buffer + m_used, m_capacity - m_used, psz);
			m_used += len;
		}

		// append one character
		void append(CharType tch)
		{
			AssertHasCapacityFor(1);
			m_buffer[m_used] = tch;
			m_buffer[++m_used] = 0;
		}

		// appends the given formatted text (see sprintf)
		void format(const CharType * formatting, ...)
		{
			va_list args;
			_crt_va_start(args, formatting);
			m_used += vformat(m_buffer + m_used, m_capacity - m_used, formatting, args);
		}

		const CharType * get_base() const { return m_buffer; }
		const CharType * get_current() const { return m_buffer + m_used; }
		size_t get_available() const { return m_capacity - m_used; }

	private:

		void AssertHasCapacityFor(size_t cch)
		{
			if (m_capacity - length() <= cch)
				throw std::range_error("buffer full");	// NOTE: unicode not compatible with std library
		}

		CharType *	m_buffer;
		size_t		m_capacity;
		size_t		m_used;
	};

	template <typename CharType>
	inline charbuffer<CharType> & operator << (charbuffer<CharType> & buffer, const CharType * psz)
	{
		buffer.append(psz);
		return buffer;
	}

	template <typename CharType>
	inline charbuffer<CharType> & operator << (charbuffer<CharType> & buffer, const CharType tch)
	{
		buffer.append(tch);
		return buffer;
	}

}