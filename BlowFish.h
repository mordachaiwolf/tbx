////////////////////////////////////////////////////////////////////////////
//
//  Blowfish.h Header File
//
//     BLOWFISH ENCRYPTION ALGORITHM
//
//     Encryption and Decryption of Byte Strings using the Blowfish Encryption Algorithm.
//     Blowfish is a block cipher that encrypts data in 8-byte blocks. The algorithm consists
//     of two parts: a key-expansion part and a data-ancryption part. Key expansion converts a
//     variable key of at least 1 and at most 56 bytes into several subkey arrays totaling
//     4168 bytes. Blowfish has 16 rounds. Each round consists of a key-dependent permutation,
//     and a key and data-dependent substitution. All operations are XORs and additions on 32-bit words.
//     The only additional operations are four indexed array data lookups per round.
//     Blowfish uses a large number of subkeys. These keys must be precomputed before any data
//     encryption or decryption. The P-array consists of 18 32-bit subkeys: P0, P1,...,P17.
//     There are also four 32-bit S-boxes with 256 entries each: S0,0, S0,1,...,S0,255;
//     S1,0, S1,1,...,S1,255; S2,0, S2,1,...,S2,255; S3,0, S3,1,...,S3,255;
//
//     The Electronic Code Book (ECB), Cipher Block Chaining (CBC) and Cipher Feedback modes
//     are used:
//
//     In ECB mode if the same block is encrypted twice with the same key, the resulting
//     ciphertext blocks are the same.
//
//     In CBC Mode a ciphertext block is obtained by first xoring the
//     plaintext block with the previous ciphertext block, and encrypting the resulting value.
//
//     In CFB mode a ciphertext block is obtained by encrypting the previous ciphertext block
//     and xoring the resulting value with the plaintext
//
//     The previous ciphertext block is usually stored in an Initialization Vector (IV).
//     An Initialization Vector of zero is commonly used for the first block, though other
//     arrangements are also in use.

#pragma once

#include "AutoMalloc.h"

#ifndef byte
using byte = unsigned char;
#endif

static_assert(sizeof(byte) == 1 && std::is_unsigned_v<byte>, "BlowFish requires that 'byte' be an 8-bit unsigned type");

namespace tbx {

	// returns the low order byte
	inline byte GetLowByte(uint32_t ui) { return (byte)(ui & 0xff); }

	class BlowFish
	{
	public:

		// Block Structure
		struct SBlock
		{
			// Constructors
			SBlock(uint32_t l = 0, uint32_t r = 0) : m_uil(l), m_uir(r) {}
			SBlock(const SBlock & roBlock) : m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) {}

			SBlock & operator^=(SBlock & b) { m_uil ^= b.m_uil; m_uir ^= b.m_uir; return *this; }

			uint32_t m_uil, m_uir;
		};

	public:
		enum Mode { ECB = 0, CBC = 1, CFB = 2 };

		// Constructor - Initialize the P and S boxes for a given Key
		BlowFish(const byte * ucKey, size_t nBytes, const SBlock & roChain = SBlock(0, 0));

		// Constructor - Initialize the P and S boxes for a given Key
		template <size_t size>
		BlowFish(const byte (&ucKey)[size], const SBlock & roChain = SBlock(0, 0)) : BlowFish(ucKey, size, roChain) {}

		// Resetting the chaining block
		void ResetChain() { m_oChain = m_oChain0; }

		// Encrypt/Decrypt Buffer in Place (nBytes must be a modulus of 8!!!)
		void Encrypt(byte * pBytes, size_t nBytes, Mode iMode = ECB);
		void Decrypt(byte * pBytes, size_t nBytes, Mode iMode = ECB);

		// Encrypt/Decrypt from Input Buffer to Output Buffer (nBytes must be a modulus of 8!!!)
		void Encrypt(const byte * in, byte * out, size_t nBytes, Mode iMode = ECB);
		void Decrypt(const byte * in, byte * out, size_t nBytes, Mode iMode = ECB);

		// Encrypt/Decrypt strings
		AutoMalloc<byte> Encrypt(std::string psz);
		std::string Decrypt(const AutoMalloc<byte> & buffer);

		// Private Functions
	private:

		uint32_t Mutate(uint32_t ui)
		{
			return ((m_auiS[0][GetLowByte(ui >> 24)] + m_auiS[1][GetLowByte(ui >> 16)]) ^ m_auiS[2][GetLowByte(ui >> 8)]) + m_auiS[3][GetLowByte(ui)];
		}

		void DoEncryption(SBlock & block);
		void DoDecryption(SBlock & block);

	private:
		// The Initialization Vector, by default {0, 0}
		SBlock m_oChain0;
		SBlock m_oChain;
		uint32_t m_auiP[18];
		uint32_t m_auiS[4][256];
		static const uint32_t scm_auiInitP[18];
		static const uint32_t scm_auiInitS[4][256];
	};


}

