// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppClangTidyCppcoreguidelinesMacroUsage
// ReSharper disable CppNonExplicitConversionOperator
#pragma once

#include <string>

#ifdef _MSC_VER
#define FW_FORCEINLINE __forceinline
#else
#define FW_FORCEINLINE __attribute__((always_inline)) inline
#endif

namespace Framework::Utilities::Strings::Fnv1A { // Fowler-Noll-Vo algorithm
	namespace detail {
		struct ValueType {
			using UnderlyingType = std::conditional_t<sizeof(std::size_t) == 8, std::uint64_t, std::uint32_t>;

			constexpr bool operator==(const ValueType &value) const noexcept {
				return this->storage == value.storage;
			}
			constexpr bool operator!=(const ValueType &value) const noexcept {
				return this->storage != value.storage;
			}
			constexpr bool operator==(ValueType &&value) const noexcept {
				return this->storage == value.storage;
			}
			constexpr bool operator!=(ValueType &&value) const noexcept {
				return this->storage != value.storage;
			}
			UnderlyingType operator*(const ValueType &value) const noexcept {
				return this->storage * value.storage;
			}
			ValueType &operator*=(const ValueType &value) noexcept {
				this->storage *= value.storage;
				return *this;
			}
			UnderlyingType operator*(ValueType &&value) const noexcept {
				return this->storage * value.storage;
			}
			ValueType &operator*=(ValueType &&value) noexcept {
				this->storage *= value.storage;
				return *this;
			}
			ValueType &operator*=(const UnderlyingType value) noexcept {
				this->storage *= value;
				return *this;
			}
			UnderlyingType operator^(const ValueType &value) const noexcept {
				return this->storage ^ value.storage;
			}
			ValueType operator^=(const ValueType &value) noexcept {
				this->storage ^= value.storage;
				return *this;
			}
			UnderlyingType operator^(ValueType &&value) const noexcept {
				return this->storage ^ value.storage;
			}
			ValueType operator^=(ValueType &&value) noexcept {
				this->storage ^= value.storage;
				return *this;
			}
			ValueType &operator^=(const UnderlyingType value) noexcept {
				this->storage ^= value;
				return *this;
			}

			constexpr FW_FORCEINLINE operator UnderlyingType() const noexcept {
				return this->storage;
			}
			
			UnderlyingType storage {};
		};
		template<class T, ValueType::UnderlyingType V>
		struct CompileTime {
			static constexpr T value = V;
		};

		constexpr ValueType prime       {sizeof(std::size_t) == 8 ? 0x100000001B3      : 0x01000193};
		constexpr ValueType offsetBasis {sizeof(std::size_t) == 8 ? 0xCBF29CE484222325 : 0x811c9dc5};
		
		template<std::size_t N>
		constexpr FW_FORCEINLINE ValueType hash(const char(&string)[N], const std::size_t length = N - 1) {
			return {(length == 1 ? static_cast<ValueType::UnderlyingType>(string[0]) ^ offsetBasis.storage : static_cast<ValueType::UnderlyingType>(string[length - 1]) ^ hash(string, length - 1).storage) * prime.storage};
		}
		constexpr FW_FORCEINLINE ValueType hash(const std::string_view::value_type character) noexcept { return {(offsetBasis ^ character) * prime}; }
		inline ValueType hash(std::string_view string) noexcept {
			auto hash = offsetBasis;
			for (const auto character : string) {
				hash ^= character;
				hash *= prime;
			}

			return hash;
		}
		template<std::size_t N>
		constexpr FW_FORCEINLINE ValueType hash(const wchar_t(&string)[N], const std::size_t length = N - 1) {
			return {(length == 1 ? static_cast<ValueType::UnderlyingType>(string[0]) ^ offsetBasis.storage : static_cast<ValueType::UnderlyingType>(string[length - 1]) ^ hash(string, length - 1).storage) * prime.storage};
		}
		constexpr FW_FORCEINLINE ValueType hash(const std::wstring_view::value_type character) noexcept { return {(offsetBasis ^ character) * prime}; }
		inline ValueType hash(std::wstring_view string) noexcept {
			auto hash = offsetBasis;
			for (const auto character : string) {
				hash ^= character;
				hash *= prime;
			}

			return hash;
		}
	}
	
	struct Hash {
		constexpr FW_FORCEINLINE Hash() noexcept = default;
		
		template<std::size_t N>
		constexpr FW_FORCEINLINE Hash(const char(&string)[N]) noexcept:
			storage {detail::hash<N>(string)} {}
		FW_FORCEINLINE Hash(const char *string) noexcept:
			storage {detail::hash(string)} {}
		explicit constexpr FW_FORCEINLINE Hash(const char character) noexcept:
			storage {detail::hash(character)} {}
		FW_FORCEINLINE Hash(const std::string_view string) noexcept:
			storage {detail::hash(string)} {}
		
		template<std::size_t N>
		constexpr FW_FORCEINLINE Hash(const wchar_t(&string)[N]) noexcept:
			storage {detail::hash<N>(string)} {}
		FW_FORCEINLINE Hash(const wchar_t *string) noexcept:
			storage {detail::hash(string)} {}
		explicit constexpr FW_FORCEINLINE Hash(const wchar_t character) noexcept:
			storage {detail::hash(character)} {}
		FW_FORCEINLINE Hash(const std::wstring_view string) noexcept:
			storage {detail::hash(string)} {}

		constexpr FW_FORCEINLINE Hash(const detail::ValueType::UnderlyingType value) noexcept:
			storage {value} {}
		
		constexpr FW_FORCEINLINE bool operator==(const Hash &hash) const noexcept {
			return this->storage == hash.storage;
		}
		constexpr FW_FORCEINLINE bool operator!=(const Hash &hash) const noexcept {
			return this->storage != hash.storage;
		}
		constexpr FW_FORCEINLINE bool operator==(Hash &&hash) const noexcept {
			return this->storage == hash.storage;
		}
		constexpr FW_FORCEINLINE bool operator!=(Hash &&hash) const noexcept {
			return this->storage != hash.storage;
		}
		constexpr FW_FORCEINLINE bool operator==(const detail::ValueType::UnderlyingType value) const noexcept {
			return this->storage == value;
		}
		constexpr FW_FORCEINLINE bool operator!=(const detail::ValueType::UnderlyingType value) const noexcept {
			return this->storage != value;
		}
		
		detail::ValueType storage {};
	};
}

#define Fnv1A(string) ( \
	Framework::Utilities::Strings::Fnv1A::detail::CompileTime< \
		Framework::Utilities::Strings::Fnv1A::detail::ValueType::UnderlyingType, \
		Framework::Utilities::Strings::Fnv1A::detail::hash(string)>::value)
