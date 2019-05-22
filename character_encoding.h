#pragma once

#include <string>

// we currently rely on deprecated features
// however, no replacement current exists...
#include <codecvt>
#include <xlocbuf>

// simple helper to convert a compile-time known ASCII narrow string to a wide string
// this does NOT take into account multi byte string encodings!
// for that you'd want to use wstring_convert or similar
// NOTE: the state of the C++ narrow-wide conversion libraries is a mess as of C++20 
//      (nothing available except in C++17, and that's been deprecated in C++20)
template <size_t size>
std::wstring ascii_to_wstring(const char(&str)[size])
{
	return std::wstring(std::begin(str), std::end(str));
}

// simple helper to convert a runtime narrow ASCII string to a wide string
inline std::wstring ascii_to_wstring(const char * psz)
{
	return std::wstring(psz, psz + std::strlen(psz));
}

// UTF-8 and wchar_t conversions
// deprecated as of c++ 17
// however, still available until a replacement is provided by the std committee
// these conversions make sense for Windows UNICODE compiled programs which store their strings using UTF-16LE - WinAPI native - encoding

// narrow() takes a wchar_t based UCS-2 or UCS-4 string and creates a utf8 encoded char based string (dep. on wchar_t)
inline std::string narrow(const wchar_t * psz) { return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(psz); }
inline std::string narrow(const std::wstring & str) { return narrow(str.c_str()); }

// narrow() an already narrow string
inline std::string narrow(const char * psz) { return psz; }
inline std::string narrow(const std::string && str) { return str; }

// widen() takes a char based UTF-8 string and creates a UCS-2 or or UCS-4 encoded wchar_t string (dep. on wchar_t)
inline std::wstring widen(const char * psz) { return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(psz); }
inline std::wstring widen(const std::string & str) { return widen(str.c_str()); }

// widen() an already wide string
inline std::wstring widen(const wchar_t * psz) { return psz; }
inline std::wstring widen(const std::wstring && str) { return str; }
