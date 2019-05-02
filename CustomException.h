#pragma once

#include <exception>
#include "character_encoding.h"

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// CCustomExeception
	//
	//	Provides a set of classes to express various error conditions
	//  The focus here is on providing an easy mechanism for the programmer
	//  to present their error message information, rather than on classifying
	//  the error by exception class.
	//
	//  One can always subclass the appropriate CCustomException subclass
	//  in order to provide a classification for their exception type.
	//
	//	We store the underlying strings using the std lib's std::runtime_error's internal mechanism (char)
	//	The assumption being that this is encoded appropriately for your needs (e.g. UTF-8)
	//	We provide wchar_t helpers that assume that the wchar_t is using native UCS-2 or -4 for the current platform
	//	which convert the encoding to UTF-8 internally
	//
	//	Please see character_encoding.h

	class CCustomException : public std::exception
	{
	public:
		// from narrow strings
		explicit CCustomException(const char * psz) : std::exception(psz) {}
		explicit CCustomException(const std::string & str) : CCustomException(str.c_str()) {}

		// from wide strings
		explicit CCustomException(const wchar_t * psz) : CCustomException(narrow(psz)) {}
		explicit CCustomException(const std::wstring & str) : CCustomException(narrow(str)) {}

		// from a std::exception
		explicit CCustomException(const std::exception & e)
			: CCustomException(e.what())
		{
		}

		// get narrow string
		const char * what() const override { return std::exception::what(); }

		// get wide string
		const std::wstring wwhat() const { return widen(what()); }
	};

	class CContextException : public CCustomException
	{
	public:
		// import ctors
		using CCustomException::CCustomException;

		// convenience: __FUNCTION__ + context message

		// narrow + narrow
		CContextException(const char * function, const std::string & message)
			: CCustomException(std::string(function) + "() : " + message)
		{
		}

		// wide + wide
		CContextException(const wchar_t * function, const std::wstring & message)
			: CCustomException(narrow(function) + "() : " + narrow(message))
		{
		}

		// narrow + wide
		CContextException(const char * function, const std::wstring & message)
			: CCustomException(std::string(function) + "() : " + narrow(message))
		{
		}

		// wide + narrow
		CContextException(const wchar_t * function, const std::string & message)
			: CCustomException(narrow(function) + "() : " + message)
		{
		}

		// convenience: __FUNCTION__ + std::exception

		// narrow + std::exception
		CContextException(const char * function, const std::exception & ex)
			: CCustomException(std::string(function) + "() : " + ex.what())
		{
		}

		// wide + std::exception
		CContextException(const wchar_t * function, const std::exception & ex)
			: CCustomException(narrow(function) + "() : " + ex.what())
		{
		}
	};


}