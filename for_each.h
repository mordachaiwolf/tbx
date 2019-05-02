#pragma once

// various for (auto e : xxxx) helpers

namespace tbx {
	namespace details {

		template <typename counting_t> struct integer_iterator
		{
			using increment_t = std::make_signed_t<counting_t>;

			counting_t value;
			increment_t by;

			integer_iterator(counting_t value, increment_t by) : value(value), by(by) {}

			counting_t operator * () const { return value; }

			bool operator == (const integer_iterator & rhs) const { return rhs.value == value; }
			bool operator != (const integer_iterator & rhs) const { return rhs.value != value; }

			auto & operator ++ () { value += by; return *this; }
			auto & operator -- () { value -= by; return *this; }

			auto & operator ++ (int) { auto prev = *this; ++*this; return prev; }
			auto & operator -- (int) { auto prev = *this; --*this; return prev; }
		};

		template <typename integer_t> struct counted_range
		{
			integer_iterator<integer_t> starting, ending;

			counted_range(integer_t start, integer_t finish, integer_t by)
				: starting(start, by)
				, ending(finish + by, by)
			{
				if ((start - finish) % by != 0)
					throw std::invalid_argument("(start - finish) % by != 0");
			}

			auto begin() const { return starting; }
			auto end() const { return ending; }

			auto rbegin() const { return integer_iterator<integer_t>(ending.value - ending.by, -ending.by); }
			auto rend() const { return integer_iterator<integer_t>(starting.value - starting.by, -starting.by); }
		};

	}

	// usage: for (auto i : counter(10)) - counts from 0 to 9 (10 total)
	template <typename integer_t> auto counter(integer_t cycles)
	{
		return details::counted_range<integer_t>(0, cycles - 1, 1);
	}

	// usage: for (auto i : counter(1, 10)) - counts from 1 to 10 (10 total)
	template <typename integer_t> auto counter(integer_t start, integer_t finish)
	{
		return details::counted_range<integer_t>(start, finish, finish < start ? -1 : 1);
	}

	// usage: for (auto i : counter(10, 2, -2) - counts from 10 to 2 by -2 (5 total)
	template <typename integer_t> auto counter(integer_t start, integer_t finish, integer_t by)
	{
		return details::counted_range<integer_t>(start, finish, by);
	}
}
	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// usage:
//	for (auto e : reversed(collection))
namespace tbx {

	namespace details {

		// simple storage of a begin and end iterator
		// allows you to turn most arbitrary iterator pairs back into a single entity that is compatible with for-each syntax
		template<typename iterator_t>
		struct iterator_range
		{
			iterator_t beginning, ending;
			iterator_range(iterator_t beginning, iterator_t ending) : beginning(beginning), ending(ending) {}

			auto begin() const { return beginning; }
			auto end() const { return ending; }

			auto rbegin() const {
				using namespace std;
				return rbegin(ending); 
			}
			auto rend() const {
				using namespace std;
				return rend(beginning);
			}
		};

	}

	template<typename T>
	auto reversed(T && collection)
	{
		using namespace std;
		return details::iterator_range(rbegin(collection), rend(collection));
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// usage:
//	for (auto e : indexed(collection))
//		e.index		// is the index
//		e.value		// is the dereferenced element (the traditional value of e)
namespace tbx {

	namespace details {

		template <typename iterator_t, typename index_t> struct indexed_iterator
		{
			iterator_t iter;
			index_t index;

			indexed_iterator(iterator_t iter, index_t index) : iter(iter), index(index) {}

			struct reference {
				using value_t = decltype(*iter);
				value_t value;
				index_t index;
			};

			auto operator * () const { return reference{ *iter, index }; }

			bool operator == (const indexed_iterator & rhs) const { return rhs.iter == iter; }
			bool operator != (const indexed_iterator & rhs) const { return rhs.iter != iter; }

			auto & operator ++ () { ++iter; ++index; return *this; }
			auto & operator -- () { --iter; --index; return *this; }

			auto & operator ++ (int) { auto prev = *this; ++*this; return prev; }
			auto & operator -- (int) { auto prev = *this; --*this; return prev; }
		};

		template <typename iterator_t, typename index_t> struct reverse_indexed_iterator
		{
			iterator_t iter;
			index_t index;

			reverse_indexed_iterator(iterator_t iter, index_t index) : iter(iter), index(index) {}

			struct reference {
				using V = decltype(*iter);
				V value;
				index_t index;
			};

			auto operator * () const { auto tmp = this->iter; return reference{ *--tmp, this->index }; }

			bool operator == (const reverse_indexed_iterator & rhs) const { return rhs.iter == iter; }
			bool operator != (const reverse_indexed_iterator & rhs) const { return rhs.iter != iter; }

			auto & operator ++ () { --iter; ++index; return *this; }
			auto & operator -- () { ++iter; --index; return *this; }

			auto & operator ++ (int) { auto prev = *this; ++*this; return prev; }
			auto & operator -- (int) { auto prev = *this; --*this; return prev; }
		};

		// simple storage of a begin and end iterator
		template<typename iterator_t, typename index_t>
		struct indexed_iterator_range
		{
			using iterator = indexed_iterator<iterator_t, index_t>;
			using reverse_iterator = reverse_indexed_iterator<iterator_t, index_t>;

			iterator beginning, ending;

			indexed_iterator_range(iterator_t beginning, iterator_t ending) : beginning(beginning, 0), ending(ending, 0) {}

			iterator begin() const { return beginning; }
			iterator end() const { return ending; }

			reverse_iterator rbegin() const { return reverse_iterator(ending.iter, 0); }
			reverse_iterator rend() const { return reverse_iterator(beginning.iter, 0); }
		};

	}

	template <typename T, typename index_t = size_t>
	auto indexed(T && collection)
	{
		using namespace std;

		auto b = begin(collection);
		auto e = end(collection);

		return details::indexed_iterator_range<decltype(b), index_t>(b, e);
	}

}