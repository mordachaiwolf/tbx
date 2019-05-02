#include "pch.h"

#define CATCH_CONFIG_MAIN	// ask catch to generate a main for us
#include "tbx\catch2.h"

#include "tbx\mfc\SimpleDropTarget.h"
#include "tbx\mfc\ApplicationInstanceRefCount.h"
#include "tbx\mutex_stream.h"
#include "tbx\core.h"
#include "tbx\for_each.h"

using namespace tbx;


SCENARIO("SimpleDropTarget")
{
	using namespace tbx::Shell;

	class TestDropTarget : public SimpleDropTarget
	{
	public:
		void OpenFilesFrom(IDataObject * pdto) { }
	};

	auto pDropTarget = std::make_unique<TestDropTarget>();

	REQUIRE(pDropTarget.get());
}

template<typename R>
bool is_ready(const std::future<R> & f)
{
	return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

SCENARIO("ApplicationInstanceRefCount")
{
	using namespace tbx::Shell;

	// test that a thread that is kept alive with an initial AddRef() until a Release() is issued...
	// subtle: the ApplicationInstanceRefCount must be created and owned 

	// this will not allow this process to terminate until our reference count drops to zero (it implements its own message loop in the dtor)
	WHEN("a thread using an ApplicationInstanceRefCount will not exit until its refcount drops to zero")
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto t2 = t1 + std::chrono::hours(1);

		std::vector<std::future<void>> futures;

		// the outer async is just there to wait for the pseudo host process
		// in order to capture the exit time (which cannot be captured within since that thread finishes its body of execution immediately
		// but doesn't terminate until the dtor for the ApplicationInstanceRefCount completes (which requires that its ref count goes to zero)
		std::async(std::launch::async, [&]{

			// launch our pseudo host process (and wait for it)
			std::async(std::launch::async, [&] {

				// we need an instance that will keep the dtor thread alive until the refcount goes to zero
				ApplicationInstanceRefCount shellRef;

				mxout << "host created with refs = 1" << std::endl;

				std::random_device dev;
				std::mt19937 rng(dev());
				std::uniform_int_distribution<std::mt19937::result_type> dist(1000, 5000); // distribution in range [1, 5]

				// let's launch several pseudo shell operations
				for (auto i : counter(1,3))
				{
					// force us to need an extra Release() call (as if a shell op is in progress)
					mxout << "shell instance " << i << " add ref " << shellRef.AddRef() << std::endl;

					// fire off our pseudo explorer process 
					// (which releases the IUnknown reference on our app ref count)
					// (which allows this pseudo host app to exit)
					futures.emplace_back(std::async(std::launch::async, [&, i] {
						// ensure that the "main" thread attempts to exit before we allow it to...
						auto t = dist(rng);
						mxout << "shell instance " << i << " sleeping for " << t << " milliseconds" << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(t));
						// release our reference to "main"
						auto refs = shellRef.Release();
						if (!refs)
						{
							// record when "explorer" finished
							t2 = std::chrono::high_resolution_clock::now();
						}
						mxout << "shell instance " << i << " release ref " << refs << std::endl;
					}));
				}

				mxout << "host attempting to exit" << std::endl;

			}).get();

			mxout << "host exited" << std::endl;

			// our inner async exited which allows us to record the current time (which arguably is later than the moment that the refcount dropped, but close)
			// note: this CAN occur before we record t2 - it's a race condition
			//       sadly, I'd have to add a mutex or similar to ensure that the release didn't allow the host thread to execute until after we set t2
			// however, this should be adequate for most runs...
			t1 = std::chrono::high_resolution_clock::now();


		}).get();

		// require that our host application didn't exit until after our simulated explorer was done with any outstanding shell operations
		REQUIRE(t1 > t2);
	}
}
