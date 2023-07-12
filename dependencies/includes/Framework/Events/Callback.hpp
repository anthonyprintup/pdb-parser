#pragma once

#include <algorithm>

#include "Priority.hpp"

namespace Framework::Events {
	template<class Callable>
	struct Callback {
		explicit Callback(Callable callable, const Priority priority = Priority::DEFAULT):
			_callable {std::move(callable)}, _priority {priority} {}

		template<typename... Arguments>
		void operator ()(Arguments&&... arguments) noexcept(
			std::is_nothrow_invocable_v<Callable, Arguments...>
		) {
			this->_callable(std::forward<Arguments>(arguments)...);
		}

		void connect() noexcept {
			this->_connected = true;
		}

		void disconnect() noexcept {
			this->_connected = false;
		}

		[[nodiscard]] bool connected() const noexcept {
			return this->_connected;
		}

		[[nodiscard]] Priority priority() const noexcept {
			return this->_priority;
		}
	private:
		Callable _callable;
		Priority _priority;
		bool     _connected {true};
	};
}