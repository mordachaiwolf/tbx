// simply delcare a class as inheriting from noncopyable to delete its ancestors copy constructor and copy operator
// hence making your class noncopyable
//
// class MyClass : protected noncopyable
// { 
//    ... 
// };

#pragma once

namespace tbx
{
	namespace details  // protection from unintended ADL
	{
		class noncopyable
		{
		protected:
			noncopyable() = default;
			~noncopyable() = default;
			noncopyable(const noncopyable&) = delete;
			noncopyable& operator=(const noncopyable&) = delete;
		};
	}

} // namespace tbx

using noncopyable = tbx::details::noncopyable;
