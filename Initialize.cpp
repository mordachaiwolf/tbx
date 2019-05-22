#include "stdafx.h"
#include "Initialize.h"

namespace tbx {

	bool IsZero(const void * ptr, size_t size)
	{
		// do lion's share using most natural/efficient size for this processor
		if (auto count = size / sizeof(unsigned); count)
		{
			auto * p = (const unsigned *)ptr;
			for (unsigned i = 0; i < count; ++i)
				if (p[i])
					return false;
		}

		// handle remainder
		if (auto rem = size % sizeof(unsigned); rem)
		{
			// TODO: profile this to see if unrolling this bit is worth it?
			if (rem == 1u)
				return *((const uint8_t *)ptr) == (uint8_t)0;
			if (rem == 2u)
				return *((const uint16_t *)ptr) == (uint16_t)0;
			if (rem == 4u)
				return *((const uint32_t *)ptr) == (uint32_t)0;

			// handle any uneven remainder as bytes
			auto * p = (const uint8_t *)ptr;
			for (unsigned i = 0; i < rem; ++i)
				if (p[i])
					return false;
		}

		return true;
	}

}
