#pragma once

#include <mutex>
#include <concepts>
#include <functional>

#include "Event.hpp"
#include "Callback.hpp"

namespace Framework::Events {
	template<class E>// requires std::derived_from<E, Event>
	struct Listener {
		static_assert(std::is_base_of_v<Event, E>, "The type passed to the listener does not derive from Event.");
		using FunctionType = std::function<void(E&)>;
		using CallbackType = Callback<FunctionType>;

		E &operator ()(E &event) {
			for (auto &&callback : this->callbacks)
				if (callback.connected())
					callback(event);

			return event;
		}

		template<typename... Arguments>
		E operator ()(Arguments&&... arguments) {
			E event {std::forward<Arguments>(arguments)...};
			return this->operator()(event);
		}

		CallbackType &connect(FunctionType function, const Priority priority = Priority::DEFAULT) {
			const auto iterator = std::find_if(
				this->callbacks.crbegin(), this->callbacks.crend(),
				[&](const CallbackType &callback) {
					return callback.priority() <= priority;
				});

			return *this->callbacks.emplace(iterator.base(), function, priority);
		}
	private:
		std::list<CallbackType> callbacks {};
	};
	
	template<class E>// requires std::derived_from<E, Event>
	struct ConcurrentListener {
		//static_assert(std::is_base_of_v<Event, E>, "The type passed to the listener does not derive from Event.");
		using FunctionType = std::function<void(E&)>;
		using CallbackType = Callback<FunctionType>;

		E &operator ()(E &event) {
			std::scoped_lock<std::mutex> lock(this->mutex);
			for (auto &&callback : this->callbacks)
				if (callback.connected())
					callback(event);

			return event;
		}

		template<typename... Arguments>
		E operator ()(Arguments&&... arguments) {
			E event {std::forward<Arguments>(arguments)...};
			return this->operator()(event);
		}

		CallbackType &connect(FunctionType function, const Priority priority = Priority::DEFAULT) {
			std::scoped_lock<std::mutex> lock(this->mutex);

			const auto iterator = std::find_if(
				this->callbacks.crbegin(), this->callbacks.crend(),
				[&](const CallbackType &callback) {
					return callback.priority() <= priority;
				});

			return *this->callbacks.emplace(iterator.base(), function, priority);
		}
	private:
		std::list<CallbackType> callbacks {};
		std::mutex mutex {};
	};
}
