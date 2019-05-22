#pragma once

#include <type_traits>
#include <cstring>
#include <cwchar>

//////////////////////////////////////////////////////////////////////////
// StringPolicy
//
//	Overview:
//
//		Essentially, a string policy provides a namespace with a standard nomenclature for dealing with various types of character encodings.
//		This also centralizes our management of C-language style zero terminated character arrays (strings).
//		So that if we need to modify our algorithms for a given character sequence encoding, we can do so in one place.
//
//	Definitions:
//
//		SU stands for a storage-unit.  This is the basic storage type for the given string.
//			ASCII	uses 7-bit bytes, mapping is fixed, although the negative codes are often in use and context-specific
//			MBCS	uses 8-bit bytes, mapping varies according to the code page in use for that string
//			UNICODE	uses a variety of SUs, mapping is universal although UTF-16 and UTF-32 come in two different endian flavors each, which changes the binary layout
//					8-bit bytes for UTF-8
//					16-bit words for UTF-16
//					32-bit words for UTF-32
//		Please note that the MS C runtime uses unsigned char for multi-byte character set functions.
//
//	Conventions:
//
//		Anytime a length is required, it is assumed to be a SU length.  Generally the length is required
//		in order to avoid accessing unallocated storage.  Hence, it is measured in SUs and NOT in glyphs!
//
//	Operations:
//
//		size_t GetLength(sz);				// returns the number of used SUs in the given string
//		size_t GetLength(sz, length);		// returns the number of used SUs in the given string (up to length)
//
//		size_t get_byte_length(sz);				// returns the number of bytes used in source string (including null terminator)
//		size_t get_byte_length(sz, length);		// returns the number of bytes used in source string (including null terminator, up to length)
//
//////////////////////////////////////////////////////////////////////////

namespace tbx {

	//////////////////////////////////////////////////////////////////////////
	// SmartChar namespace (for now, it's just an alias back to tbx, as I don't think I will be recreating the whole StringPolicy from my previous library incarnation)

	namespace SmartChar = tbx;



}
