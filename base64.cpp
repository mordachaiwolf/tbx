/*
base64.cpp and base64.h

Copyright (C) 2004-2008 René Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

René Nyffenegger rene.nyffenegger@adp-gmbh.ch

//////////////////////////////////////////////////////////////////////////

Altered by Steven Wolf, 1016-07-29

type mismatches mainly...

*/

#include "StdAfx.h"

#include "base64.h"
#include <locale>

namespace tbx {

	static const std::basic_string<unsigned char> base64_chars = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	static inline bool is_base64(unsigned char c)
	{
//		return base64_chars.find(c) != base64_chars.npos;

		// minor optimization:
		return (c >= (unsigned char)'A' && c <= (unsigned char)'Z') 
			|| (c >= (unsigned char)'a' && c <= (unsigned char)'z')
			|| (c >= (unsigned char)'0' && c <= (unsigned char)'9')
			|| (c == (unsigned char)'+')
			|| (c == (unsigned char)'/');
	}

	std::string base64_encode(unsigned char const * bytes_to_encode, size_t in_len)
	{
		std::string ret;

		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		size_t i = 0;
		while (in_len--)
		{
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		// deal with any partial triplet
		if (i)
		{
			for (auto j = i; j < 3; ++j)
				char_array_3[j] = 0;

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (auto j = 0u; j < i + 1; ++j)
				ret += base64_chars[char_array_4[j]];

			while (i++ < 3)
				ret += '=';
		}

		return ret;
	}

	std::string base64_decode(std::string const & encoded_string)
	{
		std::string ret;

		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		size_t i = 0;
		for (size_t in_ = 0, in_len = encoded_string.size(); in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]);)
		{
			char_array_4[i++] = encoded_string[in_++];
			if (i == 4)
			{
				for (i = 0; i < 4; ++i)
					char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret += char_array_3[i];
				i = 0;
			}
		}

		// deal with any partial quartet
		if (i)
		{
			for (auto j = i; j < 4; ++j)
				char_array_4[j] = 0;

			for (auto j = 0u; j < 4; ++j)
				char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (auto j = 0u; j < i - 1; ++j)
				ret += char_array_3[j];
		}

		return ret;
	}

	template <typename T>
	struct insert_pointer
	{
		insert_pointer(T * p, size_t max_count) : p_(p), max_(p + max_count) { }

		insert_pointer & operator += (const T & value)
		{
			if (p_ == max_)
				throw std::runtime_error("buffer overrun");
			*p_++ = value;
			return *this;
		}

		operator T * () { return p_; }

	private:
		T *	p_;
		T * max_;
	};

	template <typename T>
	insert_pointer<T> make_insert_pointer(T * p, size_t max_count) { return insert_pointer<T>(p, max_count); }

	void base64_decode(const char * encoded_text, size_t enc_len, unsigned char * decoded_buffer, size_t dec_len)
	{
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		auto ret = make_insert_pointer(decoded_buffer, dec_len);

		size_t i = 0;
		for (size_t in_ = 0, in_len = enc_len; in_len-- && (encoded_text[in_] != '=') && is_base64(encoded_text[in_]);)
		{
			char_array_4[i++] = encoded_text[in_++];
			if (i == 4)
			{
				for (i = 0; i < 4; ++i)
					char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret += char_array_3[i];
				i = 0;
			}
		}

		// deal with any partial quartet
		if (i)
		{
			for (auto j = i; j < 4; ++j)
				char_array_4[j] = 0;

			for (auto j = 0u; j < 4; ++j)
				char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (auto j = 0u; j < i - 1; ++j)
				ret += char_array_3[j];
		}
	}

}