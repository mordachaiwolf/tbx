#pragma once

#include <experimental/resumable>

// This code is from Raymond Chen's The Old New Thing blog (see below for links)
// no_await(some_invokable) allows us to fire and forget on a co-awaitable lambda or function
// and explicitly declare that we (this thread) doesn't care about the results
// the default policy: complete_asynchronously, does call terminate() if the result is an exception
// so that any unhandled exceptions are not simply ignored
// One could create an alternate fire and forget policy that logs the exception or handles it differently

// Raymond Chen's improved winrt::fire_and_forget
// q.v. https://devblogs.microsoft.com/oldnewthing/20190320-00/?p=102345
namespace std::experimental
{
	struct complete_asynchronously {};

	template <typename ... Args>
	struct coroutine_traits<complete_asynchronously, Args ...>
	{
		struct promise_type
		{
			complete_asynchronously get_return_object() const noexcept
			{
				return{};
			}

			void return_void() const noexcept
			{
			}

			suspend_never initial_suspend() const noexcept
			{
				return{};
			}

			suspend_never final_suspend() const noexcept
			{
				return{};
			}

			void unhandled_exception() noexcept
			{
				std::terminate();
			}
		};
	};
}

namespace tbx
{

	// no_await(any awaitable)
	// lets you specify that you're invoking an async fn w/o having this thread hold any attachments to its results at all
	// note that the newwer version of winrt::fire_and_forget does call terminate() if an unhandled exception is the result of this async
	// by default we'll use Raymond's complete_asynchronously which models that same behavior
	// q.v. https://devblogs.microsoft.com/oldnewthing/20190322-00/?p=102354
	template<typename T, typename AsyncResultPolicyType = std::experimental::complete_asynchronously>
	AsyncResultPolicyType no_await(T t)
	{
		if constexpr (std::is_invocable_v<T>)
		{
			co_await t();
		}
		else
		{
			co_await t;
		}
	}

	// invoke_async_lambda(lambda-that-returns-awaitable)
	// captures the lambda (closure), and co-awaits on it
	// Now you can use it for async lambdas that return any kind of awaitable object, 
	// like a Concurrency::task<int>, or a winrt::Windows::Foundation::IAsync­Action, or a std::experimental::future<std::string>. 
	// And since it returns the resulting coroutine, you can continue operating with it.
	// q.v. https://devblogs.microsoft.com/oldnewthing/20190118-00/?p=100735
	template<typename T>
	auto invoke_async_lambda(T lambda) -> decltype(lambda())
	{
		// note: I would think we want T&& to get a moved instance of lambda, not T which gives us a copied instance of lambda...
		return co_await lambda();
	}

}
