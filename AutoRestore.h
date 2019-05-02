#pragma once

// an AutoRestorer overrides the value of some entity and restores it when it goes out of scope
// it's a bit like a ScopeGuard, where the guard is to restore the overridden value.
//
// Usage:
//	auto restoreMainWnd = make_autorestorer(m_pMainWnd, &dlg);


#include "noncopyable.h"
//#include "SmartChar.h"	// for the char* wchar_t* specializations, we rely upon SmartChar - which the caller will need to have included for this aspect of this facility to work

#include <xutility>		// std::move



namespace tbx {

	namespace details {

		class AutoRestorerBase : protected noncopyable
		{
		public:
			virtual ~AutoRestorerBase() = default;

			void Discard() const noexcept { m_discarded = true; }
			void Restore() const noexcept { if (!m_discarded) RestoreValue(); }

		protected:
			AutoRestorerBase() noexcept : m_discarded(false) { }
			AutoRestorerBase(AutoRestorerBase && rhs) : m_discarded(std::move(rhs.m_discarded)) {}

			virtual void RestoreValue() const noexcept = 0;
			mutable bool m_discarded;
		};

		template<typename T>
		class AutoRestorer : public AutoRestorerBase
		{
		public:
			// save the value to be restored on our destruction & temporarily override its value
			AutoRestorer(T & target, const T & temporary_value)
				: m_target(target)
				, m_original_value(target)
			{
				target = temporary_value;
			}

			// save the value to be restored on our destruction (we don't actually override the value at all)
			explicit AutoRestorer(T & target)
				: m_target(target)
				, m_original_value(target)
			{
			}

			// move an auto restorer (allows return from function)
			AutoRestorer(AutoRestorer<T> && rhs)
				: AutoRestorerBase(std::move(rhs))
				, m_target(std::move(rhs._target))
				, m_original_value(std::move(rhs._original_value))
			{
			}

			~AutoRestorer() override { Restore(); }

			const T & original_value() const { return m_original_value; }

		protected:
			void RestoreValue() const noexcept override
			{
				m_target = m_original_value;
				m_discarded = true;
			}

		private:
			T &		m_target;
			T		m_original_value;
		};


		// specialization for string buffers
		template <size_t size>
		class AutoRestorer<char[size]> : public AutoRestorerBase
		{
		public:
			typedef char string_buffer[size];

			// save the value to be restored on our destruction & temporarily override its value
			AutoRestorer(string_buffer & target, const char * temporary_value) : m_target(target)
			{
				copy(m_original_value, m_target);
				copy(m_target, temporary_value);
			}

			// save the value to be restored on our destruction (we don't actually override the value at all)
			explicit AutoRestorer(string_buffer & target) : m_target(target)
			{
				copy(m_original_value, m_target);
			}

			// move an auto restorer (allows return from function)
			AutoRestorer(AutoRestorer<char[size]> && rhs)
				: AutoRestorerBase(std::move(rhs))
				, m_target(std::move(rhs._target))
				, m_original_value(std::move(rhs._original_value))
			{
			}

			~AutoRestorer() override { Restore(); }

		protected:
			void RestoreValue() const noexcept override
			{
				copy(m_target, m_original_value);
				m_discarded = true;
			}

		private:
			string_buffer &	m_target;
			string_buffer	m_original_value;
		};

		// specialization for string buffers
		template <size_t size>
		class AutoRestorer<wchar_t[size]> : public AutoRestorerBase
		{
		public:
			typedef wchar_t string_buffer[size];

			// save the value to be restored on our destruction & temporarily override its value
			AutoRestorer(string_buffer & target, const wchar_t * temporary_value) : m_target(target)
			{
				copy(m_original_value, m_target);
				copy(m_target, temporary_value);
			}

			// save the value to be restored on our destruction (we don't actually override the value at all)
			explicit AutoRestorer(string_buffer & target) : m_target(target)
			{
				copy(m_original_value, m_target);
			}

			// move an auto restorer (allows return from function)
			AutoRestorer(AutoRestorer<wchar_t[size]> && rhs)
				: AutoRestorerBase(std::move(rhs))
				, m_target(std::move(rhs._target))
				, m_original_value(std::move(rhs._original_value))
			{
			}

			~AutoRestorer() override { Restore(); }

		protected:
			void RestoreValue() const noexcept override
			{
				copy(m_target, m_original_value);
				m_discarded = true;
			}

		private:
			string_buffer &	m_target;
			string_buffer	m_original_value;
		};

	} // namespace details

	// make_autorestore(entity, override)
	template <typename T, typename V>
	auto make_autorestore(T & target, V && newvalue)
	{
		return details::AutoRestorer<T>{ target, std::forward<V>(newvalue) };
	}

	// make_autorestore(entity)
	// this variation saves & restores the value but doesn't immediately override it
	template <typename T>
	auto make_autorestore(T & target)
	{
		return details::AutoRestorer<T>{ target };
	}

} // namespace tbx
