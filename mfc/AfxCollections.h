// AfxCollections.h

#pragma once

#include <functional>
#include <afxstr.h>

namespace tbx {

	// extend countof() to include Afx style collections that supply GetSize()
	template <typename T> auto countof(const T & collection) -> decltype(collection.GetSize()) { return collection.GetSize(); }

// 	// extend countof() to include Afx style collections that supply GetLength()
// 	template <typename T> auto countof(const T & collection) -> decltype(collection.GetLength()) { return collection.GetLength(); }

	//////////////////////////////////////////////////////////////////////////
	// traits to provide access to contained types
	//////////////////////////////////////////////////////////////////////////

	// Maps
	template <typename T>
	struct afx_map_traits : protected T
	{
		using key_type = typename T::BASE_KEY;
		using key_arg_type = typename T::BASE_ARG_KEY;
		using value_type = typename T::BASE_VALUE;
		using value_arg_type = typename T::BASE_ARG_VALUE;
	};

	// Arrays and Lists
	template <typename T>
	struct afx_collection_traits : protected T
	{
		using value_type = typename T::BASE_TYPE;
		using arg_type = typename T::BASE_ARG_TYPE;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	// MFC collection classes utilities
	///////////////////////////////////////////////////////////////////////////////////////

	// inserts each element of the collection separated by a separator
	template<typename OutputType, typename IteratorType, typename SeparatorType>
	void Compose(OutputType & out, const IteratorType & begin, const IteratorType & end, SeparatorType separator)
	{
		if (begin == end)
			return;

		out << *begin;

		IteratorType i = begin;
		while (++i, i != end)
		{
			out << separator;
			out << *i;
		}
	}

	// inserts each element of the collection separated by a separator
	template<typename OutputType, typename CollectionType, typename SeparatorType>
	void Compose(OutputType & out, const CollectionType & collection, SeparatorType separator)
	{
		for (auto z = countof(collection), i = 0; i < z; ++i)
		{
			if (i)
				out << separator;
			out << collection[i];
		}
	}

	// returns a composition from the elements in the given collection, separated by separator
	template<typename CompositionType, typename CollectionType, typename SeparatorType>
	CompositionType GetComposition(const CollectionType & collection, const SeparatorType & separator)
	{
		CompositionType composition;
		Compose(composition, collection, separator);
		return composition;
	}

	// returns a CString composed from the elements in the given collection, separated by separator
	template<typename CollectionType, typename SeparatorType>
	CString GetComposedString(const CollectionType & collection, const SeparatorType & separator)
	{
		return GetComposition<CString>(collection, separator);
	}

	// convert a character delimited string of substrings into an arbitrary MFC collection (or anything with an Add() interface)
	template<typename CollectionType>
	void AddSubstrings(CollectionType & collection, const CString & strSubstrings, TCHAR chDelimeter)
	{
		int nStart = 0;
		int nEnd = strSubstrings.Find(chDelimeter);
		const int nLength = strSubstrings.GetLength();
		while (nStart < nLength)
		{
			if (nEnd == -1)
				nEnd = nLength;

			collection.Add(strSubstrings.Mid(nStart, nEnd - nStart));

			nStart = nEnd + 1;
			nEnd = strSubstrings.Find(chDelimeter, nStart);
		}
	}

	// convert a character delimited string of substrings into an arbitrary MFC collection (or anything with an Add() interface)
	template<typename CollectionType, typename T_CONVERTER>
	void AddSubstrings(CollectionType & collection, T_CONVERTER conversion_functor, const CString & strSubstrings, TCHAR chDelimeter)
	{
		int nStart = 0;
		int nEnd = strSubstrings.Find(chDelimeter);
		const int nLength = strSubstrings.GetLength();
		while (nStart < nLength)
		{
			if (nEnd == -1)
				nEnd = nLength;

			collection.Add(conversion_functor(strSubstrings.Mid(nStart, nEnd - nStart)));

			nStart = nEnd + 1;
			nEnd = strSubstrings.Find(chDelimeter, nStart);
		}
	}

	// convert a character delimited string of substrings into an arbitrary MFC collection (or anything with an Add() interface)
	template<typename CollectionType>
	void AddSubstrings(CollectionType & collection, const CString & strSubstrings, const CString & strDelimeters)
	{
		int nStart = 0;
		int nEnd = 0;
		const int nLength = strSubstrings.GetLength();
		while (nStart < nLength)
		{
			while (strDelimeters.Find(strSubstrings[nStart]) != -1)
			{
				if (++nStart == nLength)
					return;
			}

			nEnd = nStart + 1;
			while (strDelimeters.Find(strSubstrings[nEnd]) == -1)
			{
				if (++nEnd == nLength)
					break;
			}

			ASSERT(nEnd > nStart);

			collection.Add(strSubstrings.Mid(nStart, nEnd - nStart));

			nStart = nEnd + 1;
		}
	}

	// find the given entity within a collection
	// returns index of element, or -1 if not found
	//   collection must support operator []
	template<typename CollectionType, typename T, class PredicateType>
	int Find(const CollectionType & collection, const T & element, PredicateType predicate)   // returns index or -1
	{
		for (auto z = countof(collection), i = 0; i < z; ++i)
		{
			if (predicate(collection[i], element))
				return i;
		}
		return -1;
	}

	// find the given entity within a collection
	// returns index of element, or -1 if not found
	//   collection must support operator []
	//   element must support ==
	template<typename CollectionType, typename T>
	int Find(const CollectionType & collection, const T & element)    // returns index or -1
	{
//		// note: this isn't as good because std::equal_to is forcing both sides of the == to be of type T&, when it may be something else depending on how the collection's operator[] works...
//		return Find(collection, element, std::equal_to<const T &>());

		for (auto z = countof(collection), i = 0; i < z; ++i)
		{
			if (collection[i] == element)
				return i;
		}
		return -1;
	}

	// find the given entity within a collection
	// returns index of element, or -1 if not found
	//   collection must support operator []
	template<typename CollectionType, typename T, class PredicateType>
	int ReverseFind(const CollectionType & collection, const T & element, PredicateType predicate)
	{
		for (auto i = countof(collection); i; )
		{
			if (predicate(collection[--i], element))
				return i;
		}
		return -1;
	}

	// find the given entity within a collection
	// returns index of element, or -1 if not found
	//   collection must support GetSize() and []
	//   element must support ==
	template<typename CollectionType, typename T>
	int ReverseFind(const CollectionType & collection, const T & element)
	{
		for (auto i = countof(collection); i; )
		{
			if (collection[--i] == element)
				return i;
		}
		return -1;
	}

	// returns true if given entity exists in the collection
	//  see Find() above for additional notes...
	template<typename CollectionType, typename T, class PredicateType>
	bool Contains(const CollectionType & collection, const T & element, PredicateType predicate)  // returns true if found
	{
		return Find(collection, element, predicate) != -1;
	}

	// returns true if given entity exists in the collection
	//  see Find() above for additional notes...
	template<typename CollectionType, typename T>
	bool Contains(const CollectionType & collection, const T & element)   // returns true if found
	{
		return Find(collection, element) != -1;
	}

	// returns true if any value in the map meets predicate criteria
	template <class PredicateType>
	bool Contains(const CMapStringToString & collection, const CString & element, PredicateType predicate)
	{
		for (auto pos = collection.GetStartPosition(); pos; )
		{
			CString key, value;
			collection.GetNextAssoc(pos, key, value);
			if (predicate(value, element))
				return true;
		}
		return false;
	}

	// returns true if any value in the map is equal to element
	inline bool Contains(const CMapStringToString & collection, const CString & element)   // returns true if found
	{
		return Contains(collection, element, std::equal_to<CString>());
	}

	//////////////////////////////////////////////////////////////////////////

	// returns the position of the first element that meets the predicates conditions
	template<typename MapType, typename T, typename PredicateType>
	typename afx_map_traits<MapType>::key_type ReverseLookup(const MapType & collection, const T & element, PredicateType predicate)
	{
		afx_map_traits<MapType>::key_type key;
		afx_map_traits<MapType>::value_type value;
		for (auto pos = collection.GetStartPosition(); pos; )
		{
			collection.GetNextAssoc(pos, key, value);
			if (predicate(value, element))
				return key;
		}
		throw CContextException("Element not found");
	}

	template<typename MapType, typename T>
	typename afx_map_traits<MapType>::key_type ReverseLookup(const MapType & collection, const T & element)
	{
		return ReverseLookup(collection, element, std::equal_to<afx_map_traits<MapType>::value_type>());
	}


	///////////////////////////////////////////////////////////////////////////////////////
	// Basic type conversions to and from byte arrays
	///////////////////////////////////////////////////////////////////////////////////////


	// Convert follows the form: Convert(from, to)
	template <typename T>
	void Convert(T & fromSimpleData, CByteArray & toByteArray)
	{
		toByteArray.SetSize(sizeof(fromSimpleData));
		for (int i = 0; i < sizeof(fromSimpleData); ++i)
		{
			toByteArray[i] = ((BYTE*)(&fromSimpleData))[i];
		}
	}

	// Convert follows the form: Convert(from, to)
	template <typename T>
	void Convert(CByteArray & fromByteArray, T & toSimpleData)
	{
		const int arraySize = fromByteArray.GetSize();
		if (arraySize != sizeof(toSimpleData))
			throw CContextException(__FUNCTION__, _T("cannot convert from a CByteArray: count mismatch"));
		for (int i = 0; i < sizeof(toSimpleData); ++i)
			((byte*)(&toSimpleData))[i] = fromByteArray[i];
	}

}