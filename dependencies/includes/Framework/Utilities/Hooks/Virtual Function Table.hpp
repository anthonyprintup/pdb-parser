// ReSharper disable CppClangTidyCppcoreguidelinesSpecialMemberFunctions
// ReSharper disable CppClangTidyReadabilityQualifiedAuto
// ReSharper disable CppNonExplicitConvertingConstructor
#pragma once

#include <memory>
#include <variant>

namespace Framework::Utilities::Hooks {
	namespace detail {
		template<class T>
		concept IsFunction = std::is_pointer_v<T> && std::is_function_v<std::remove_pointer_t<T>>;
		/*template<class T>
		concept IsStatelessLambda = requires(T lambda) {
			lambda.operator();
			std::is_empty_v<T>;
		};*/

		inline std::size_t functionTableSize(const std::uintptr_t functionTable) noexcept {
			std::size_t size {};
			for (; reinterpret_cast<std::uintptr_t*>(functionTable)[size]; ++size) {}
			return size;
		}
	}
	
	struct VirtualFunctionTable {
		VirtualFunctionTable() noexcept = default;
		VirtualFunctionTable(const std::uintptr_t pointer):
			object {pointer} {
			const auto table = this->functionTable = *reinterpret_cast<std::uintptr_t*>(pointer);
			const auto functionTableSize = detail::functionTableSize(table) + 1; // size + type information pointer

			using UnderlyingType = std::unique_ptr<std::uintptr_t[]>;
			const auto replacement = std::get<UnderlyingType>(this->replacement = std::make_unique<std::uintptr_t[]>(functionTableSize)).get();
			std::memcpy(replacement,
						reinterpret_cast<std::uintptr_t*>(table - sizeof(std::uintptr_t)),
						functionTableSize * sizeof(std::uintptr_t)); // type information + function table
		}
		VirtualFunctionTable(const std::uintptr_t pointer, std::uintptr_t replacementBuffer) noexcept:
			object {pointer} {
			const auto table = this->functionTable = *reinterpret_cast<std::uintptr_t*>(pointer);
			const auto functionTableSize = detail::functionTableSize(table) + 1; // size + type information pointer

			this->replacement = replacementBuffer;
			std::memcpy(reinterpret_cast<std::uintptr_t*>(replacementBuffer),
						reinterpret_cast<std::uintptr_t*>(table - sizeof(std::uintptr_t)),
						functionTableSize * sizeof(std::uintptr_t)); // type information + function table
		}
		~VirtualFunctionTable() {
			this->restore();
		}
		
		VirtualFunctionTable(const VirtualFunctionTable&) = delete;
		VirtualFunctionTable(VirtualFunctionTable&&)      = delete;
		VirtualFunctionTable &operator=(const std::uintptr_t pointer) {
			this->object = pointer;
			const auto functionTable = this->functionTable = *reinterpret_cast<std::uintptr_t*>(pointer);
			const auto functionTableSize = detail::functionTableSize(functionTable) + 1; // size + type information pointer

			using UnderlyingType = std::unique_ptr<std::uintptr_t[]>;
			const auto replacement = std::get<UnderlyingType>(this->replacement = std::make_unique<std::uintptr_t[]>(functionTableSize)).get();
			std::memcpy(replacement,
						reinterpret_cast<std::uintptr_t*>(functionTable - sizeof(std::uintptr_t)),
						functionTableSize * sizeof(std::uintptr_t)); // type information + function table
			
			return *this;
		}

		[[nodiscard]] std::uintptr_t original(const std::size_t index) const noexcept {
			return reinterpret_cast<std::uintptr_t*>(this->functionTable)[index];
		}
		template<class CallbackType>
		[[nodiscard]] CallbackType original(const std::size_t index) const noexcept requires detail::IsFunction<CallbackType> {
			return reinterpret_cast<CallbackType>(this->original(index));
		}
		std::uintptr_t hook(const std::uintptr_t callback, const std::size_t index) noexcept {
			if (!this->object || !this->functionTable || this->replacement.valueless_by_exception() || index >= detail::functionTableSize(this->functionTable))
				return {};
			if (const auto replacementBuffer = std::get_if<std::uintptr_t>(&this->replacement))
				replacementBuffer[index + 1] = callback;
			else if (const auto replacementArray = std::get_if<std::unique_ptr<std::uintptr_t[]>>(&this->replacement))
				replacementArray->get()[index + 1] = callback;
			return this->original(index);
		}
		template<class CallbackType>
		auto hook(CallbackType &&callback, const std::size_t index) noexcept requires detail::IsFunction<CallbackType> {
			return reinterpret_cast<std::decay_t<CallbackType>>(this->hook(reinterpret_cast<std::uintptr_t>(callback), index));
		}

		void replace() const noexcept {
			if (!this->object || !this->functionTable || this->replacement.valueless_by_exception())
				return;
			
			if (const auto replacementBuffer = std::get_if<std::uintptr_t>(&this->replacement))
				*reinterpret_cast<std::uintptr_t*>(this->object) = *replacementBuffer + sizeof(std::uintptr_t);
			else if (const auto replacementArray = std::get_if<std::unique_ptr<std::uintptr_t[]>>(&this->replacement))
				*reinterpret_cast<std::uintptr_t*>(this->object) = reinterpret_cast<std::uintptr_t>(replacementArray->get()) + sizeof(std::uintptr_t);
		}
		void restore() const noexcept {
			if (!this->object || !this->functionTable)
				return;
			*reinterpret_cast<std::uintptr_t*>(this->object) = this->functionTable;
		}
	private:
		using VariantType = std::variant<std::uintptr_t, std::unique_ptr<std::uintptr_t[]>>;
		std::uintptr_t
			object {},
			functionTable {};
		VariantType replacement {};
	};
}
