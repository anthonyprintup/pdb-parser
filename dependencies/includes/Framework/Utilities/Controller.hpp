#pragma once

#include <list>
#include <mutex>
#include <memory>
#include <concepts>
#include <algorithm>

#include "../Utilities/Type Information.hpp"

namespace Framework::Utilities {
	namespace detail {
		template<class U, class T>
		concept Derived = std::derived_from<U, T>;
	}
	
	template<class T>
	struct Controller {
		template<detail::Derived<T> U, class... Arguments>
		U *add(Arguments&&... arguments) {
			if (const auto resolved = this->template find<U>())
				return resolved;

			constexpr auto typeHash = internal::Type<U>::hash();
			const auto &[hash, object] = this->objects.emplace_front(
				typeHash, typename Container::second_type(
					new U(std::forward<Arguments>(arguments)...),
					[](void *pointer) { delete static_cast<U*>(pointer); }));

			return static_cast<U*>(object.get());
		}

		template<detail::Derived<T> U>
		[[nodiscard]] U *get() const {
			return this->template find<U>();
		}

		template<detail::Derived<T> U>
		void remove() {
			this->objects.remove_if([](const auto &container) {
				return container.storage == internal::Type<U>::hash();
			});
		}
	protected:
		using Container = std::pair<Strings::Fnv1A::detail::ValueType::UnderlyingType, std::unique_ptr<T, void(*)(void*)>>;
		std::list<Container> objects {};
	private:
		template<detail::Derived<T> U>
		[[nodiscard]] U *find() const {
			if (const auto &iterator = std::find_if(this->objects.cbegin(), this->objects.cend(), [](const auto &container) {
				return container.first == internal::Type<U>::hash();
			}); iterator != this->objects.cend())
				return static_cast<U*>(iterator->second.get());

			return nullptr;
		}
	};

	template<class T>
	struct ConcurrentController {
		template<detail::Derived<T> U, class... Arguments>
		U *add(Arguments&&... arguments) {
			if (const auto resolved = this->template find<U>())
				return resolved;

			constexpr auto typeHash = internal::Type<U>::hash();
			const auto &[hash, object] = this->objects.emplace_front(
				typeHash, Container::second_type(
					new U(std::forward<Arguments>(arguments)...),
					[](void *pointer) { delete static_cast<U*>(pointer); }));

			return static_cast<U*>(object.get());
		}

		template<detail::Derived<T> U>
		[[nodiscard]] U *get() const {
			return this->template find<U>();
		}

		template<detail::Derived<T> U>
		void remove() {
			std::scoped_lock lock {this->mutex};
			this->objects.remove_if([](const auto &container) {
				return container.hash == internal::Type<U>::hash();
			});
		}
	protected:
		using Container = std::pair<Strings::Fnv1A::detail::ValueType::UnderlyingType, std::unique_ptr<T, void(*)(void*)>>;
		std::list<Container> objects {};
		
		std::mutex mutex {};
	private:
		template<detail::Derived<T> U>
		[[nodiscard]] U *find() const {
			std::scoped_lock lock {this->mutex};
			if (const auto &iterator = std::find_if(this->objects.cbegin(), this->objects.cend(), [](const auto &container) {
				return container.first == internal::Type<U>::hash();
			}); iterator != this->objects.cend())
				return static_cast<U*>(iterator->second.get());

			return nullptr;
		}
	};
}