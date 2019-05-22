#pragma once
#include <type_traits>
#include "CustomException.h"

//////////////////////////////////////////////////////////////
// Raw Memory Initialization Helpers
//
//	Provides:
//		Zero(x) where x is any type, and is completely overwritten by null bytes (0).
//		Initialized<T> x; where T is any legacy type, and it is completely null'd before use.
//
// History:
//
//	User UncleBen of stackoverflow.com and I tried to come up with 
//	an improved, integrated approach to Initialized<>
//	http://stackoverflow.com/questions/2238197/how-do-i-specialize-a-templated-class-for-data-type-classification
//
//	In the end, there are simply some limitations to using this
//	approach, which makes it... limited.
//
//	For the time being, I have integrated them as best I can
//	However, it is best to simply use this feature
//	for legacy structs and not much else.
//
//	So I recommend stacked based usage for legacy structs in particular:
//		Initialized<BITMAP> bm;
//
//	And perhaps some very limited use legacy arrays:
//		Initialized<TCHAR[MAX_PATH]> filename;
//
//	But I would discourage their use for member variables:
//		Initialized<size_t> m_cbLength;
//	...as this can defeat template type deduction for such types 
//	(its not a size_t, but an Initialized<size_t> - different types!)
//
//////////////////////////////////////////////////////////////


namespace tbx {

	// zero the memory space of a given PODS or native array
	template <typename T>
	void Zero(T & object, int zero_value = 0)
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "Type must be trivial");

		// make zeroing out a raw pointer illegal
		static_assert(!std::is_pointer_v<T>, "Zeroing out a raw pointer is illegal");

		std::memset(&object, zero_value, sizeof(object));
	}

	// version for simple arrays
	template <typename T, size_t size>
	void Zero(T(&object)[size], int zero_value = 0)
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "Cannot overwrite a non-trivial class");

		std::memset(&object, zero_value, sizeof(object));
	}

	// version for dynamically allocated memory
	template <typename T>
	T * Zero(T * object, size_t size, int zero_value = 0)
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "Cannot overwrite a non-trivial class");
		std::memset(object, zero_value, size);
		return object;
	}

	// returns true if the given memory range is all zeroed out
	bool IsZero(const void * ptr, size_t size);

	// throws if the given block of memory isn't zeroed out
	void VerifyZero(const void * ptr, size_t nbytes)
	{
		if (!IsZero(ptr, nbytes))
			throw CContextException(__FUNCTION__, "memory is not zero");
	}

	// throws if the given array's space isn't all zero
	template <typename T, size_t size>
	void VerifyZero(const T (&ptr)[size])
	{
		// ensure that we aren't trying to overwrite non-trivial elements
		static_assert(std::is_pod_v<T>, "contents must be trivial");

		VerifyZero(ptr, size * sizeof(T));
	}

	// throws if the given object's space isn't all zero
	// NOTE: do not use const T - see https://stackoverflow.com/questions/27971374/how-do-i-get-a-template-to-favor-t-over-t/27971520#27971520
	template <
		typename T,
		std::enable_if_t<!std::is_pointer_v<T>, int> = 0
	>
	void VerifyZero(T & ref)
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "type must be trivial");

		VerifyZero(&ref, sizeof(T));
	}

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Initialized for non-inheritable types
	// usage: Initialized<int> i;
	template <typename T, bool SCALAR = std::is_scalar_v<T>>
	struct Initialized
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_scalar_v<T>, "This template specialization is only for a scalar type");

		// the data
		T	m_value;

		// default valued construction
		Initialized() : m_value() { }

		// implicit valued construction (auto-conversion)
		template <typename U> Initialized(const U & rhs) : m_value(rhs) { }

		// assignment
		template <typename U> T & operator = (const U & rhs) { if ((void*)&m_value != (void*)&rhs) m_value = rhs; return *this; }

		// implicit conversion to the underlying type
		operator T & () { return m_value; }
		operator const T & () const { return m_value; }

		// address-of operator
		T * operator & () { return &m_value; }
		const T * operator & () const { return &m_value; }

		// explicit accessor to simple type
		T & get() { return m_value; }
		const T & get() const { return m_value; }

		// zero method for this type
		void zero() { m_value = T(); }
	};

	//////////////////////////////////////////////////////////////////////////
	// Initialized for inheritable types (e.g. structs)
	// usage:  Initialized<RECT> r;
	template <typename T>
	struct Initialized<T, false> : public T
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "Can't overwrite a non-trivial class");

		// inherit all constructors
		using T::T;

		// add a default ctor
		Initialized() : T() {  }

		// auto-conversion ctor
		template <typename OtherType> Initialized(const OtherType & value) : T(value) { }

		// auto-conversion assignment
		template <typename OtherType> Initialized & operator = (const OtherType & value) { static_cast<T &>(*this) = value; return *this; }

		// explicit address-of operator
		T * operator & () { return this; }
		const T * operator & () const { return this; }

		// explicit accessor to simple type
		T & get() { return *this; }
		const T & get() const { return *this; }

		// zero method for this type
		void zero() { Zero((T &)(*this)); }
	};

	//////////////////////////////////////////////////////////////////////////
	// Initialized arrays of simple types
	// usage: Initialized<char[MAXFILENAME]> szFilename;
	template <typename T, size_t size>
	struct Initialized<T[size], false>
	{
		// ensure that we aren't trying to overwrite a non-trivial class
		static_assert(std::is_pod_v<T>, "Can't overwrite a non-trivial class");

		// internal data
		T m_array[size];

		// default ctor
		//Initialized() : m_array() { }	// Generates a warning about new behavior.  Its okay, but might as well not produce a warning.
		Initialized() { Zero(m_array); }

		// implicit array access
		operator T * () { return m_array; }
		operator const T * () const { return m_array; }

		// explicit address-of operator
		T * operator & () { return m_array; }
		const T * operator & () const { return m_array; }

		// explicit accessor to simple type
		T * get() { return m_array; }
		const T * get() const { return m_array; }

		// NOTE: All of the following techniques leads to ambiguity.
		//       Sadly, allowing the type to convert to ArrayType &, which IMO should
		//		 make it fully "the same as it was without this wrapper" instead causes
		//       massive confusion for the compiler (it doesn't understand IA + offset, IA[offset], etc.)
		//       So in the end, the only thing that truly gives the most bang for the buck is T * conversion.
		//       This means that we cannot really use this for <char> very well, but that's a fairly small loss
		//       (there are lots of ways of handling character strings already)

		// 	// automatic conversions
		// 	operator ArrayType & () { return m_array; }
		// 	operator const ArrayType & () const { return m_array; }
		// 
		// 	T * operator + (long offset) { return m_array + offset; }
		// 	const T * operator + (long offset) const { return m_array + offset; }
		// 
		// 	T & operator [] (long offset) { return m_array[offset]; }
		// 	const T & operator [] (long offset) const { return m_array[offset]; }

		// metadata
		size_t GetCapacity() const { return size; }

		// zero method for this type
		void zero() { Zero(m_array); }
	};

	//////////////////////////////////////////////////////////////////////////
	// Initialized for pointers to simple types
	// usage: Initialized<char*> p;
	// Please use a real smart pointer (such as std::unique_ptr or std::shared_ptr)
	//	instead of this template whenever possible.  This is really a stop-gap for legacy
	//	code, not a comprehensive solution.
	template <typename T>
	struct Initialized<T*, true>
	{
		// the pointer
		T *	m_pointer;

		// default valued construction
		Initialized() : m_pointer(nullptr) { }

		// valued construction (auto-conversion)
		template <typename U> Initialized(const U * rhs) : m_pointer(rhs) { }

		// assignment
		template <typename U> T * & operator = (U * rhs) { if (m_pointer != rhs) m_pointer = rhs; return *this; }
		template <typename U> T * & operator = (const U * rhs) { if (m_pointer != rhs) m_pointer = rhs; return *this; }

		// implicit conversion to underlying type
		operator T * & () { return m_pointer; }
		operator const T * & () const { return m_pointer; }

		// explicit accessor to simple type
		T * get() { return m_pointer; }
		const T * get() const { return m_pointer; }

		// pointer semantics
		const T * operator -> () const { return m_pointer; }
		T * operator -> () { return m_pointer; }
		const T & operator * () const { return *m_pointer; }
		T & operator * () { return *m_pointer; }

		// allow null assignment
	private:
		class Dummy {};
	public:
		// amazingly, this appears to work.  The compiler finds that Initialized<T*> p = null to match the following definition

		T * & operator = (Dummy * value) { m_pointer = nullptr; ASSERT(value == nullptr); return *this; }

		// zero method for this type
		void zero() { m_pointer = nullptr; }
	};

	//////////////////////////////////////////////////////////////////////////
	// Uninitialized<T> requires that you explicitly initialize it when you declare it (or in the owner object's ctor)
	//  it has no default ctor - so you *must* supply an initial value.
	template <typename T>
	struct Uninitialized
	{
		// valued initialization
		Uninitialized(T initial_value) : m_value(initial_value) { }

		// valued initialization from convertible types
		template <typename U> Uninitialized(const U & initial_value) : m_value(initial_value) { }

		// assignment
		template <typename U> T & operator = (const U & rhs) { if (&m_value != &rhs) m_value = rhs; return *this; }

		// implicit conversion to underlying type
		operator T & () { return m_value; }
		operator const T & () const { return m_value; }

		// explicit accessor to simple type
		T & get() { return m_value; }
		const T & get() const { return m_value; }

		// the data
		T	m_value;
	};

	//////////////////////////////////////////////////////////////////////////
	// Zero() overload for Initialized<>
	//////////////////////////////////////////////////////////////////////////

	// version for Initialized<T>
	template <typename T, bool B>
	void Zero(Initialized<T, B> & object)
	{
		object.zero();
	}

}