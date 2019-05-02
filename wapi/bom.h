#pragma once

// #define NOMINMAX
// #include <wtypes.h>

namespace Toolbox {

	constexpr BYTE kUTF8[] = { 0xEF, 0xBB, 0xBF };	// NOTE: don't normally use this - the assumption should be UTF8 without BOM
	constexpr BYTE kUTF16LE[] = { 0xFF, 0xFE };
	constexpr BYTE kUTF16BE[] = { 0xFE, 0xFF };
	constexpr BYTE kUTF32LE[] = { 0xFF, 0xFE, 0x00, 0x00 };
	constexpr BYTE kUTF32BE[] = { 0x00, 0x00, 0xFE, 0xFF };

	enum class BOM
	{
		none,
		utf8,
		utf16le,
		utf16be,
		utf32le,
		utf32be,
		invalid,
	};

	// diagnostic message (English verbose name)
	const TCHAR * GetName(BOM bom);

	// returns the byte count of a given BOM
	size_t GetLength(BOM bom);

	// returns the bytes and their length of a given BOM
	void GetBOM(BOM bom, const BYTE * & pBytes, size_t & cbBytes);

	// returns the type of BOM this buffer starts with (or none)
	BOM HasBOM(const BYTE * buffer, size_t cbBuffer);

	// returns the type of BOM this buffer starts with (or none)
	template <typename T, size_t size>
	BOM HasBOM(const T(&buffer)[size]) { return HasBOM((const BYTE*)buffer, size * sizeof(T)); }

	// true if the buffer starts with a UTF16LE BOM
	inline bool IsUTF16LE(const BYTE * buffer, size_t cbBuffer) { return HasBOM(buffer, cbBuffer) == BOM::utf16le; }

	//////////////////////////////////////////////////////////////////////////

	// creates a text file with only the BOM in it (i.e. it marks it as being a UTFx file, but writes nothing else to it)
	// it only succeeds if it doesn't exist, and we have permission to create the specified file
	// default BOM = UTF16 little endian, which is the Windows internal representation, and the only one compatible with WritePrivateProfileStringW)
	// NOTE: we don't currently support UTF8 BOMs
	bool CreateTextFile(const TCHAR * filename, bool bOverwriteExisting, BOM bom = BOM::utf16le);

	// reads the first WORD of the specified file to detect if it has a BOM 
	// returns true if the file could be read from, bom contains the value of the first WORD
	// NOTE: we don't currently support UTF8 BOMs
	bool ReadBOM(const TCHAR * filename, BOM & bom);

}