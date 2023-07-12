#pragma once

#include <random>

// ReSharper disable once CppInconsistentNaming
namespace Framework::Utilities::RandomNumberGenerators {
	namespace detail {
		template<class T>
		concept Arithmetic = std::is_arithmetic_v<T>;
		template<class T>
		concept Integral = std::is_integral_v<T>;

		__forceinline constexpr std::uint64_t compileTimeSeed() {
			constexpr auto time = __TIME__;
			constexpr auto seed = static_cast<std::uint64_t>(
				time[7]        +
				time[6] * 10   +
				time[4] * 60   +
				time[3] * 600  +
				time[1] * 3600 +
				time[0] * 360000);

			return seed;
		}
	}

	template<detail::Arithmetic T>
	T mersenneTwister(std::mt19937_64 &randomizer,
					  const T minimum, const T maximum) {
		return static_cast<T>(std::uniform_real_distribution<>(static_cast<double>(minimum), static_cast<double>(maximum))(randomizer));
	}
	
	template<detail::Arithmetic T>
	T mersenneTwister(const std::random_device::result_type device,
					  const T minimum, const T maximum) {
		std::mt19937_64 randomizer {device};
		return mersenneTwister(randomizer, minimum, maximum);
	}
	
	template<detail::Arithmetic T>
	T mersenneTwister(const T minimum, const T maximum) {
		const auto randomDevice = std::random_device {}();
		return mersenneTwister(randomDevice, minimum, maximum);
	}

	template<detail::Integral T, std::size_t Iterations>
	__forceinline constexpr T compileTimeGenerator(const T minimum, const T maximum) {
		struct {
			constexpr __forceinline auto generate() {
				const auto previousState {container.state};
				container.state = previousState * 6364136223846793005ULL + (container.seed | 1);

				const auto XOR = ((previousState >> 18U) ^ previousState) >> 27U;
				// ReSharper disable once CppInconsistentNaming
				const auto ROT = previousState >> 59U;

				return XOR >> ROT | XOR << (ROT & 31);
			}
			
			struct {
				std::uint64_t state {};
				std::uint64_t seed  {detail::compileTimeSeed()};
			} container {};
		} generator {};

		for (auto i {Iterations}; i > 0; --i)
			generator.generate();

		return static_cast<T>((generator.generate() % maximum) + minimum);
	}
}
