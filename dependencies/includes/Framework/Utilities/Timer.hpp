#pragma once

#include <chrono>

namespace Framework::Utilities {
	namespace detail {
		template<class T>
		concept Duration = std::chrono::_Is_duration_v<T>;
	}
	
	template<detail::Duration ClockType>
	class Timer { // TODO: figure out if std::chrono::time_point would be a better implementation
		constexpr static bool nothrow {std::is_arithmetic_v<typename ClockType::rep>};
	public:
		// _Is_duration_v is a replacement for std::chrono::is_clock_v, which is only available in c++2a

		Timer() noexcept(this->nothrow):
			_time {this->epoch()} {}
		
		[[nodiscard]] auto time() const noexcept(this->nothrow) {
			return (this->epoch() - this->_time).count();
		}

		[[nodiscard]] bool reached(const std::size_t duration) const noexcept(this->nothrow) {
			return this->epoch() - this->_time >= static_cast<ClockType>(duration);
		}
		
		void reset(const std::size_t delay = 0) noexcept(this->nothrow) {
			this->_time = this->epoch() - static_cast<ClockType>(delay);
		}
	private:
		ClockType _time {};

		[[nodiscard]] auto epoch() const noexcept(this->nothrow) {
			return std::chrono::duration_cast<ClockType>(std::chrono::high_resolution_clock::now().time_since_epoch());
		}
	};

	struct MsTimer: Timer<std::chrono::milliseconds> {};
}