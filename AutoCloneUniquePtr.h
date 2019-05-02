#pragma once

#include <memory>

// adapted with gratitude from http://stackoverflow.com/questions/23726228/auto-cloning-unique-ptr
// very simple idea: just use clone method for copies
//
// WARNING: this does open the door to unintended copies
//			but honestly, for cheaply copyable objects, this is nicer than having to invade the host and manually define the copy for every niggling member
//
// To use: simply declare a polymorphic hiararchy that includes a base class with a virtual clone() member to do the real cloning of an instance
//
// q.v. cloneable.h
//
// define TRACE_AUTO_CLONE to expose the tracing code (uses MFC's WinAPI debugging wrappers)

#if !defined(TRACE_AUTO_CLONE) && !defined(Trace)
#define Trace(...) 
#endif

namespace tbx 
{

	template <typename T>
	class auto_cloned_unique_ptr : public std::unique_ptr<T>
	{
	public:
		// default ctor
		auto_cloned_unique_ptr() = default;

		// copy constructor (your type must supply the clone() virtual method!)
		auto_cloned_unique_ptr(const auto_cloned_unique_ptr<T> & rhs)
			: std::unique_ptr<T>(rhs ? rhs->clone() : nullptr) 
		{
			if (rhs)
				Trace(_T("%hs - cloning %p\n"), __FUNCTION__, rhs.get());
		}

		// copy assignment
		auto_cloned_unique_ptr<T> & operator = (const auto_cloned_unique_ptr<T> & rhs)
		{
			if (rhs)
			{
				reinterpret_cast<std::unique_ptr<T>&>(*this) = rhs->clone();
				Trace(_T("%hs - cloning %p\n"), __FUNCTION__, rhs.get());
			}
			else
				reset();
			return *this;
		}

		// move ctor
		auto_cloned_unique_ptr(auto_cloned_unique_ptr<T> && rhs)
			: std::unique_ptr<T>(std::move(rhs))
		{
			if (get())
				Trace(_T("%hs - moving %p\n"), __FUNCTION__, get());
		}

		// move assignment
		auto_cloned_unique_ptr<T> & operator = (auto_cloned_unique_ptr<T> && rhs)
		{
			if (rhs)
				Trace(_T("%hs - moving %p\n"), __FUNCTION__, rhs.get());
			std::unique_ptr<T>::operator = ((std::unique_ptr<T> &&)rhs);
			return *this;
		}

		// move conversion ctor
		auto_cloned_unique_ptr(std::unique_ptr<T> ptr)
			: std::unique_ptr<T>(std::move(ptr)) 
		{
			if (get())
				Trace(_T("%hs - move-converting %p\n"), __FUNCTION__, get());
		}

		// move conversion assignment
		auto_cloned_unique_ptr<T> & operator = (std::unique_ptr<T> && rhs)
		{
			if (rhs)
				Trace(_T("%hs - move-converting %p\n"), __FUNCTION__, rhs.get());
			std::unique_ptr<T>::operator = (std::move(rhs));
			return *this;
		}

	};

}
