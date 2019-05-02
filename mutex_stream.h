#pragma once

// inspired by: https://stackoverflow.com/questions/4446484/a-line-based-thread-safe-stdcerr-for-c/53288135#53288135
// simply buffers all insertions to a string stream, and then flushes that to the underlying output stream
// synchronized by a per-output stream global mutex

#include <iostream>
#include <sstream>
#include <mutex>
#include <map>

// we cannot use global instances as we need to create and destroy our instance in order to take and release the lock each time
#define mxerr tbx::mutex_stream(std::cerr)
#define mxout tbx::mutex_stream(std::cout)

namespace tbx
{
	class mutex_stream : public std::ostringstream
	{
		static inline std::mutex g_mutex;
		static inline std::unordered_map<void*, std::mutex> g_mutexes;

		static std::mutex & get_mutex(std::ostream & os) 
		{
			std::lock_guard<std::mutex> lock(g_mutex);
			return mutex_stream::g_mutexes[&os]; 
		}

	public:
		mutex_stream(std::ostream & os) 
			: m_stream(os)
			, m_mutex(get_mutex(os))
		{
			// lock our stream while we read attributes from it (otherwise it could be in an incoherent state with other threads writing to it)
			std::lock_guard lock(m_mutex);

			// copyfmt causes odd problems with lost output (so don't use for now)
//			copyfmt(os);

			// copy whatever properties are relevant
			imbue(os.getloc());
			precision(os.precision());
			width(os.width());

			// initial reasonable defaults
			setf(std::ios::fixed, std::ios::floatfield);
		}

		~mutex_stream()
		{
#ifdef __cpp_lib_uncaught_exceptions
			// do NOT try to flush ourselves if we're unwinding an exception!
			if (std::uncaught_exceptions())
				return;
#else
			// do NOT try to flush ourselves if we're unwinding an exception!
			if (std::uncaught_exception())
				return;
#endif

			// and don't bother if we're in a bad state
			if (good())
			{
				// build the string before taking the lock
				auto s = str();

				// only output the string if non-empty
				if (!s.empty())
				{
					// note: we're breaking the cardinal rule to never throw during a dtor
					//       we'll throw if the underlying stream throws for the insertion operator
					std::lock_guard<std::mutex> lock(m_mutex);
					m_stream << std::move(s);
				}
			}
		}

	private:
		std::ostream & m_stream;
		std::mutex & m_mutex;
	};
}
