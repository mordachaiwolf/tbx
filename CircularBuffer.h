#pragma once
#include <stdexcept>

//////////////////////////////////////////////////////////////////////////
// CircularBuffer
//
//	Provides for a circular buffer (rolling buffer) of any arbitrary element type
//
//////////////////////////////////////////////////////////////////////////

namespace tbx {


	template <size_t minimum, size_t maximum>
	struct CircularCounter
	{
		static_assert(minimum < maximum, "CircularCounter<> require a non-empty ordered range");
	public:
		static constexpr auto onecycle = maximum - minimum + 1;

		// constructors
		CircularCounter() : value(minimum) { };
		explicit CircularCounter(size_t startvalue) : value(minimum + (startvalue - minimum) % onecycle) { };

		// the current value
		size_t get() const { return value; }
		//operator size_t () const { return value; }	// this opens us up to converting to a simple type and then having ops that work on that applied, instead of limiting us to just the ops we explicitly support

		// the pure offset
		size_t offset() const { return value - minimum; }

		// comparison operations
		bool operator == (const CircularCounter<minimum, maximum> & rhs) const { return value == rhs.value; }
		bool operator != (const CircularCounter<minimum, maximum> & rhs) const { return value != rhs.value; }

		bool operator == (size_t rhs) const { return value == rhs; }
		bool operator != (size_t rhs) const { return value != rhs; }

		// circular math (wraps around the "clock")
		size_t operator + (size_t diff) const
		{
			return minimum + ((offset() + diff) % onecycle);
		}
		size_t operator - (size_t diff) const
		{
			diff %= onecycle;
			if (offset() >= diff)
				return minimum + (offset() - diff);
			else
				return minimum + (offset() + onecycle - diff);
		}

		size_t operator + (const CircularCounter<minimum, maximum> & rhs) const
		{
			return *this + rhs.offset();
		}
		size_t operator - (const CircularCounter<minimum, maximum> & rhs) const
		{
			return *this - rhs.offset();
		}

		// increment & decrement
		CircularCounter & operator ++ ()
		{
			Increment();
			return *this;
		}

		CircularCounter operator ++ (int)
		{
			CircularCounter copy(*this);
			Increment();
			return copy;
		}

		CircularCounter & operator -- ()
		{
			Decrement();
			return *this;
		}

		CircularCounter operator -- (int)
		{
			CircularCounter copy(*this);
			Decrement();
			return copy;
		}

		void Increment()
		{
			if (value == maximum)
				value = minimum;
			else
				++value;
		}

		void Decrement()
		{
			if (value == minimum)
				value = maximum;
			else
				--value;
		}

	private:
		size_t value;
	};


	//////////////////////////////////////////////////////////////////////////

	struct overflow_bad_policy
	{
		void operator () ()
		{
			throw std::overflow_error("CircularBuffer::Write() - buffer full!");
		}
	};

	struct overflow_wrap_policy
	{
		void operator () ()
		{
			// do nothing here - it's simply not considered an error to continue to write w/o first reading the data
		}
	};


	struct underflow_bad_policy
	{
		void operator () ()
		{
			throw std::underflow_error("CircularBuffer::Read() - buffer empty!");
		}
	};


	template <typename element_type, size_t length, class overflow_policy = overflow_bad_policy, class underflow_policy = underflow_bad_policy>
	class CircularBuffer
	{
	public:

		// NOTE: do !!!NOT!!! declare a zero sized CircularBuffer!!!  It will have undefined results
		static_assert(length, "Cannot declare a zero sized CircularBuffer!!!  It would have undefined results!!!");

		// types
		using index_t = CircularCounter<0, length - 1>;

		// size (which is static)
		static constexpr size_t size() { return length; }

		// constructors
		CircularBuffer() : full(false) { }

		// state
		bool IsFull() const { return (read_index == write_index) && full; }
		bool IsEmpty() const { return (read_index == write_index) && !full; }

		// attributes

		// returns the number of elements available for reading
		size_t GetCount() const
		{
			// handle full or empty
			if (read_index == write_index)
				return full ? length : 0;

			// anything else
			return write_index - read_index;
		}

		void Reset()
		{
			full = false;
			read_index = write_index = index_t();
		}

		// operations

		// write one element to the end of the buffer
		void Write(const element_type & elem)
		{
			// overflow
			if (IsFull())
				overflow_policy()();

			// write the element
			buffer[write_index.get()] = elem;

			// update our indices
			if (IsFull())
				++read_index, ++write_index;
			else
				full = (++write_index == read_index);
		}

		// read & remove one element from the head of the buffer
		const element_type & Read()
		{
			// underflow
			if (IsEmpty())
				underflow_policy()();
			full = false;
			const auto index = read_index++;	// use the previous position to return the value
			return buffer[index.get()];	// we cannot use buffer[read_index++] because the returned type is not a valid scalar
		}

		// accessors

		// view one element at the given offset from the current head of the buffer (0..count-1)
		// NOTE: NOT THE ABSOLUTE HEAD OF THE BUFFER!!!
		const element_type & operator [] (size_t offset) const
		{
			if (offset >= length)
				throw std::range_error("CircularBuffer::[] index out of bounds!");
			return buffer[read_index + offset];
		}

	private:
		bool			full;				// indicates the meaning of read_index == write_index;
		index_t			write_index;		// index to write next element to
		index_t			read_index;			// index to read next element from
		element_type	buffer[length];		// the actual buffer
	};


} // namespace
