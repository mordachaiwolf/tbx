// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#define _CONVERSION_DONT_USE_THREAD_LOCALE	// force CP_ACP *not* CP_THREAD_ACP for MFC CString auto-conveters!!!

#define NOMINMAX			// Don't define min & max as macros (Bad Microsoft)
#include <windows.h>

// GDI+
#pragma warning(push)
#pragma warning(disable:4458)	// declaration of 'symbol' hides class member - tons of these in the 8.1 SDK - MS needs to fix their code (as usual)
#include <minmax.h>				// gdiplus.h requires min & max macros
#include <gdiplus.h>
#undef min						// but I hate them with a passion
#undef max						// so lets not pollute our caller's code with them
#pragma warning(pop)

// reference additional headers your program requires here

// stdlib
#include <memory>
#include <string>

// tbx
#include "tbx/core.h"
