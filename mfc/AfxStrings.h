#pragma once


namespace tbx::mfc {

	// adapter to use a CString for target of a back_insert_iterator (i.e. an output iterator)
	template <typename cstring_t>
	struct cstring_back_insert_iterator
	{
		using container_type = cstring_t;
		using char_t = typename cstring_t::XCHAR;

		// construct us on a 
		cstring_back_insert_iterator(container_type & str) : m_string(str) {}

		// what we actually do
		cstring_back_insert_iterator & operator = (char_t c) { m_string += c; return *this; }

		// these are all no-ops for back inserter's
		cstring_back_insert_iterator & operator * () { return *this; }
		cstring_back_insert_iterator & operator ++ () { return *this; }
		cstring_back_insert_iterator & operator ++ (int) { return *this; }

		// efficiency extension: insert a whole run of characters...
		cstring_back_insert_iterator & operator = (const char_t * psz) { m_string += psz; return *this; }

	private:
		container_type &	m_string;	// where we're accumulating our data
	};

	template <typename cstring_t>
	cstring_back_insert_iterator<cstring_t> make_back_inserter(cstring_t & str)
	{
		return cstring_back_insert_iterator<cstring_t>(str);
	}

}
