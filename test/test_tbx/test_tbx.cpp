#include "pch.h"

#define CATCH_CONFIG_MAIN	// ask catch to generate a main for us
#include "..\catch2.h"

#include "tbx\AutoRestore.h"
#include "tbx\base64.h"
#include "tbx\BitTest.h"
#include "tbx\BufferedAdaptor.h"
#include "tbx\character_encoding.h"
#include "tbx\CircularBuffer.h"
#include "tbx\core.h"
#include "tbx\CustomException.h"
#include "tbx\for_each.h"
#include "tbx\noawait.h"
#include "tbx\AutoMalloc.h"
#include "tbx\AutoStringBuffer.h"
#include "tbx\BlowFish.h"

using namespace tbx;

SCENARIO("autorestore swaps in a value and then restores the original value at scope exit")
{
	// normally has "goodbye"
	std::string s = "goodbye";
	if (true)
	{
		// override to have "hello"
		auto override = make_autorestore(s, "hello");
		// output "hello"
		REQUIRE(s == "hello");
	}
	// output "goodbye"
	REQUIRE(s == "goodbye");
}

SCENARIO("base 64 encodes and decodes binary sequences symmetrically")
{
	uint8_t bytes[] = { 0x88, 0xA4, 0x1F, 0x3B, 0xFF, 0x20 };
	auto e = base64_encode(bytes, countof(bytes));
	auto s = base64_decode(e);
	REQUIRE(memcmp(bytes, s.data(), countof(bytes)) == 0);
}

SCENARIO("ascii_to_wstring() converts ASCII encoded strings to wstring representation")
{
	REQUIRE(ascii_to_wstring(__FUNCTION__).compare(__FUNCTIONW__) == 0);
}

SCENARIO("narrow() and widen() converts between UTF8 and wchar_t string representations (which could be UTF-16 or UTF-32, depending on the platform)")
{
	const wchar_t kSeasons[] = L"春夏冬秋";
	REQUIRE(widen(narrow(kSeasons)) == std::wstring(kSeasons));
}

SCENARIO("buffered adaptor")
{
	int value = 9;
	auto ba = make_buffered_adaptor(value);
	GIVEN("a buffered adaptor on an 'int' set to 9")
	{
		*ba = 55;
		WHEN("it is set to 55")
		{
			THEN("it's value is 55, but the underlying value is 9")
			{
				REQUIRE(*ba == 55);
				REQUIRE(value == 9);
			}
		}

		WHEN("it is applied")
		{
			ba.apply();
			THEN("the real value is updated to 55")
			{
				REQUIRE(value == 55);
			}
		}
	}
}

SCENARIO("CircualCounter")
{
	GIVEN("a circular counter from 0 to 11")
	{
		CircularCounter<0, 11> c;
		WHEN("it is incremented 12 times, it wraps around")
		{
			for (int i = 0; i < 12; ++i)
			{
				REQUIRE(c == i);
				++c;
			}
			REQUIRE(c == 0);
		}
	}

	GIVEN("a circular counter from 5 to 10")
	{
		CircularCounter<5, 10> c;
		WHEN("6 is added to it, it wraps back around to 5")
		{
			REQUIRE(c + 6 == 5);
		}
		WHEN("3 is subtracted from it, it wraps to 8")
		{
			REQUIRE(c - 3 == 8);
		}
	}
}

SCENARIO("CircularBuffer")
{
	GIVEN("a circual buffer of length 100 with an overflow-allowed policy...")
	{
		const auto kMaxElems = 100u;
		CircularBuffer<int, kMaxElems, overflow_wrap_policy> buffer;

		WHEN("100 elements are added")
		{
			for (auto c = kMaxElems; c; --c)
				buffer.Write(rand());

			THEN("100 elements are available")
			{
				REQUIRE(buffer.GetCount() == kMaxElems);
				REQUIRE(buffer.IsFull());
			}

			WHEN("50 elements are read")
			{
				for (auto c = 0; c < kMaxElems / 2; ++c)
					buffer.Read();

				THEN("50 elements remain")
				{
					REQUIRE(buffer.GetCount() == kMaxElems / 2);
				}

				WHEN("100 additional elements are added")
				{
					for (int i = 0; i < kMaxElems;)
						buffer.Write(++i);

					THEN("those 100 are available in the same order")
					{
						for (int i = 0; i < kMaxElems;)
							REQUIRE(++i == buffer.Read());

						THEN("and now the buffer is empty")
						{
							REQUIRE(buffer.IsEmpty());
						}
					}
				}
			}
		}
	}
}

SCENARIO("Clonable class hierarchies can be cloned")
{
	// an arbitrary clonable class hierarchy
	struct FormatData
	{
		virtual ~FormatData() = default;
		virtual std::unique_ptr<FormatData> clone() = 0;
	};

	struct FormatDecimalData : FormatData	// x -> %.*f
	{
		FormatDecimalData(int decimals) : decimals(decimals) {}

		std::unique_ptr<FormatData> clone() override {
			return std::make_unique<FormatDecimalData>(*this);
		}

		bool operator == (const FormatDecimalData & rhs)
		{
			return decimals == rhs.decimals;
		}

		int		decimals;
	};

	struct FormatPercentData : FormatData	// x -> %.*f (but x is multiplied by 100)
	{
		FormatPercentData(int decimals) : decimals(decimals) {}

		std::unique_ptr<FormatData> clone() override {
			return std::make_unique<FormatPercentData>(*this);
		}

		bool operator == (const FormatPercentData & rhs)
		{
			return decimals == rhs.decimals;
		}

		int		decimals;
	};

	struct FormatFractionData : FormatData
	{
		FormatFractionData(int denominator, double tolerance, int decimals)
			: denominator(denominator)
			, tolerance(tolerance)
			, decimals(decimals)
		{
		}

		std::unique_ptr<FormatData> clone() override {
			return std::make_unique<FormatFractionData>(*this);
		}

		bool operator == (const FormatFractionData & rhs)
		{
			return denominator == rhs.denominator
				&& tolerance == rhs.tolerance
				&& decimals == rhs.decimals;
		}

		int			denominator;
		double		tolerance;
		int			decimals;
	};

	struct FormatAreaData : FormatData
	{
		FormatAreaData(int scale, int decimals) : scale(scale), decimals(decimals) {}

		std::unique_ptr<FormatData> clone() override {
			return std::make_unique<FormatAreaData>(*this);
		}

		bool operator == (const FormatAreaData & rhs)
		{
			return scale == rhs.scale && decimals == rhs.decimals;
		}

		int		scale;
		int		decimals;
	};

	GIVEN("a pointer to an instance of a clonable class")
	{
		std::unique_ptr<FormatData> p = std::make_unique<FormatAreaData>(1, 2);

		WHEN("it is cloned through the a pointer to base")
		{
			auto p2 = p->clone();
			THEN("we end up with a valid copy of the subclass through the base pointer as a new base pointer")
			{
				bool bTest = ((FormatAreaData &)*p.get()) == ((FormatAreaData&)*p2.get());
				REQUIRE(bTest);
			}
		}
	}
}

SCENARIO("Bit Manipulation Templates")
{
	// simple equality (a == b)
	REQUIRE(BitCompare((int8_t)32, 32u));
	REQUIRE_FALSE(BitCompare(32l, 30u));
	REQUIRE(BitCompare((uint16_t)0xFFFF, (uint16_t)-1));
	REQUIRE_FALSE(BitCompare((uint16_t)0xFFFF, -1));

	constexpr auto original_value = 0x0100010010000001;
	auto b = original_value;

	// simple equality for a sub-range of bits (a & mask == b & mask)
	REQUIRE(BitCompare((uint16_t)0xFFFF, (uint16_t)-1));
	REQUIRE_FALSE(BitCompare((uint16_t)0xFFFF, -1));

	// and-test non-zero (a & b != 0)
	REQUIRE(BitTest(b, 1));	// lowest bit
	REQUIRE(BitTest(b, -1));	// -1 => 0xFFFFFFFF (32 bits on most compilers)

	// and-test equal (a & b == b)
	REQUIRE_FALSE(BitTestAll(b, 0x0F));

	// equal sub-range (a & mask == b & mask)
	REQUIRE(BitCompare(b, b, (uint16_t)b));

	// clear (a &= ~b)
	BitClear(b, 0x01);
	REQUIRE(b == 0x0100010010000000);

	// set (a |= b)
	BitSet(b, 0xF0);
	REQUIRE(b == 0x01000100100000F0);

	// set sub-range to a specified value (a = a & ~mask | b & mask)
	BitSetTo(b, 0xF0, 0x1CF);
	REQUIRE(b == 0x01000100100000C0);

	// get a random value
	auto r = rand();
	BitSetIf(b, original_value, r & 1);
	if (r & 1)
		REQUIRE(b == original_value);
	else
		REQUIRE_FALSE(b == original_value);
}

SCENARIO("CContextException can be constructed from any combination of narrow or wide strings, as well as from other std::exceptions")
{
	CContextException ex1(__FUNCTION__, "narrow, narrow");
	CContextException ex2(__FUNCTION__, L"narrow, wide");
	CContextException ex3(__FUNCTIONW__, "wide, narrow");
	CContextException ex4(__FUNCTIONW__, L"wide, wide");
	CContextException ex5(ex1);
	std::bad_alloc ba;
	CContextException ex6(ba);
}

// SCENARIO("noawait")
// {
// 	invoke_async_lambda([] { return sqrt(3.3); });
// }

SCENARIO("counter() allows simple for-each syntax when you simply want a looping index from a start to end value, optionally with an arbitrary increment value")
{
	int i;

	i = 0;
	for (auto e : counter(5))
		REQUIRE(e == i++);
	REQUIRE(i == 5);
	
	i = 0;
	for (auto e : counter(1, 5))
		REQUIRE(e == ++i);
	REQUIRE(i == 5);

	i = 12;
	for (auto e : counter(10, 0, -2))
	{
		i -= 2;
		REQUIRE(e == i);
	}
	REQUIRE(i == 0);

	i = 0;
	for (auto e : counter(3, 0))
	{
		REQUIRE(e == 3 - i++);
	}
	REQUIRE(i == 4);
}

SCENARIO("reversed() allows for-each syntax to use reverse iterators for the given iterable object")
{
	std::vector<int> collection{ 5, 9, 15, 22 };
	auto i = countof(collection);
	for (auto e : reversed(collection))
		REQUIRE(e == collection[--i]);

	long values[] = { 3, 6, 9, 12 };
	i = countof(values);
	for (auto e : reversed(values))
		REQUIRE(e == values[--i]);

	auto bc = reversed(counter(1, 5));
	REQUIRE(*bc.begin() == 5);
	REQUIRE(*bc.end() == 0);

	i = 6;
	for (auto e : bc)
		REQUIRE(e == --i);
	REQUIRE(i == 1);
}

SCENARIO("indexed() allows for a simple way to get both the entity and the index to the entity (always counting from 0)")
{
	for (auto e : indexed(counter(5)))
		REQUIRE(e.value == e.index);		// 0/0, 1/1, .. 4/4

	for (auto e : indexed(counter(1, 5)))
		REQUIRE(e.value == 1 + e.index);	// 1/0, 2/1, .. 5/4

	for (auto e : indexed(counter(10, 0)))
		REQUIRE(e.value == 10 - e.index);	// 10/0, 9/1, .. 0/10

	auto bx = reversed(indexed(counter(5)));
	auto beg = *bx.begin();
	REQUIRE(beg.index == 0);
	REQUIRE(beg.value == 4);
	for (auto e : bx)
		REQUIRE(e.value == 4 - e.index);	// 4/0, 3/1, .. 0/4

	for (auto e : reversed(indexed(counter(1, 5))))
		REQUIRE(e.value == 5 - e.index);	// 5/0, 4/1, .. 1/4

	for (auto e : reversed(indexed(counter(10, 0))))
		REQUIRE(e.value == e.index);		// 0/0, 1/1, .. 10/10
}

SCENARIO("AutoMalloc")
{
	AutoMalloc<unsigned> am;
	GIVEN("an AutoMalloc<unsigned>")
	{
		WHEN("initially created, it does not allocate any buffer")
		{
			REQUIRE(am.size_in_bytes() == 0);
			REQUIRE(am.get() == nullptr);
		}
		WHEN("resized to 100, it has 100 unsigned's worth of storage")
		{
			am.realloc(100);
			REQUIRE(am.size() == 100);
			REQUIRE(am.size_in_bytes() == 100 * sizeof(unsigned));

			WHEN("it is moved into another host, it no longer has any associated buffer")
			{
				AutoMalloc<unsigned> am2(std::move(am));
				REQUIRE(am2.size() == 100);
				REQUIRE(am.size() == 0);
				REQUIRE(am.get() == nullptr);
				am = std::move(am2);
			}

			WHEN("it can be moved into a host with an orthogonal size in bytes to the original")
			{
				AutoMalloc<char> am2;
				am2.take_cast(std::move(am));
				REQUIRE(am2.size() == 100 * sizeof(unsigned));
			}
		}
	}
}

SCENARIO("AutoStrBuffer")
{
	const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
	const char helloworld[] = "hello world";
	std::string s(helloworld);
	GIVEN("an AutoStrBuffer<> of lengthof(alphabet) length")
	{
		THEN("the entire alphabet can be written to it, including the final null")
		{
			AutoStrBuffer<> asb(s, lengthof(alphabet));
			REQUIRE(asb.size() == countof(alphabet));
			memcpy(asb.get(), alphabet, lengthof(alphabet));
			REQUIRE(memcmp(asb.get(), alphabet, countof(alphabet)) == 0);

			THEN("it can be moved into another AutoStrBuffer without side effects to the original string")
			{
				// force abs2 to destroy at the end of this scope, causing s to update
				{
					AutoStrBuffer<> asb2(std::move(asb));
					REQUIRE(s == helloworld);
					REQUIRE(!asb.HasOwnership());
					REQUIRE(asb2.HasOwnership());
					REQUIRE(asb2.size() == countof(alphabet));
					REQUIRE(memcmp(asb2.get(), alphabet, countof(alphabet)) == 0);
				}

				THEN("when the autobuffer is destroyed, the original string takes on the buffer")
				{
					REQUIRE(s == alphabet);
				}
			}
		}
	}
}

SCENARIO("BlowFish")
{
	const byte key[] = { 0x22, 0x3C, 0x8A, 0xFF, 0xE0, 0xC3, 0x99, 0xFA, 0x03, 0x59, 0xA1, 0xBB };
	BlowFish cypher(key);
	WHEN("a string is encrypted")
	{
		const char plaintext[] = "the bad fox ducked under the barbed wire fence";
		auto data = cypher.Encrypt(plaintext);
		THEN("the data can be decrypted using the same cypher-key")
		{
			auto str = cypher.Decrypt(data);
			REQUIRE(str == plaintext);
		}
	}
}