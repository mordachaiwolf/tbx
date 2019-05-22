#pragma once

#include <exception>
#include "core.h"
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

	// previous projects had a separate CLabeledException that is now merged into CContextException
	using CLabeledException = CContextException;

}

// !! is used to ensure that any overloaded operators used to evaluate 'expr' do not end up at &&.
#define TBX_ASSERT_MESSAGE(expr, msg) (void)((!!expr) || (throw CContextException(__FUNCTION__, MESSAGE(msg)), true))
#define TBX_ASSERT_ALWAYS(expr) TBX_ASSERT_MESSAGE((expr), _STRINGIZE(expr))
#define TBX_ASSERT(expr) TBX_ASSERT_ALWAYS(expr)

// simply throw a context exception for now
// at some point I can revisit all of these and possibly convert them to C++20 precondition / postcondition contracts
#define TBX_PRECONDITION(expr) TBX_ASSERT_MESSAGE((expr), "precondition violated: " _STRINGIZE(expr))
#define TBX_POSTCONDITION(expr) TBX_ASSERT_MESSAGE((expr), "postcondition violated: " _STRINGIZE(expr))

// TBX_ASSERT_DEBUG_ONLY is only executed in a debug build (it is elided for release builds)
#ifdef _DEBUG
#define TBX_ASSERT_DEBUG_ONLY(expr) TBX_ASSERT_ALWAYS(expr)
#else
#define TBX_ASSERT_DEBUG_ONLY(expr) 
#endif // _DEBUG

// TBX_VERIFY always executes, but only throws on failure in a debug build
#ifdef _DEBUG
#define TBX_VERIFY(expr) TBX_ASSERT(expr)
#else
#define TBX_VERIFY(expr) (expr)
#endif
