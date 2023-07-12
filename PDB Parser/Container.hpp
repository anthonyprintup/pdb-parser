#pragma once

#include "msf/Record.hpp"
#include "msf/Types/Structure.hpp"
#include "msf/Types/Union.hpp"
#include "msf/Types/Enum.hpp"
#include "msf/Types/Bit Field.hpp"
#include "msf/Types/Array.hpp"

#include <vector>
#include <unordered_map>
#include <variant>

namespace pdb {
	struct Container {
		std::vector<const msf::codeview::Record*> records {};
		std::pair<std::size_t, std::size_t> typeIndexRange {};
	
		std::unordered_map<std::size_t, msf::types::Structure> structures {};
		std::unordered_map<std::size_t, std::pair<msf::HashType, const msf::types::Structure*>> forwardedStructures {};
		std::unordered_map<std::size_t, msf::types::Union> unions {};
		std::unordered_map<std::size_t, std::pair<msf::HashType, const msf::types::Union*>> forwardedUnions {};
		std::unordered_map<std::size_t, msf::types::Enum> enums {};
		std::unordered_map<std::size_t, std::pair<msf::HashType, const msf::types::Enum*>> forwardedEnums {};
		std::unordered_map<std::size_t, msf::types::BitField> bitFields {};
		std::unordered_map<std::size_t, msf::types::Array> arrays {};
	
		void generateStructures();
		void resolveForwardDeclarations();
	
		[[nodiscard]] bool isPointer(std::size_t typeIndex) const noexcept;
		[[nodiscard]] msf::UInt32 resolvePointerTypeIndex(std::size_t typeIndex) const noexcept;
	private:
		[[nodiscard]] bool isClassType(std::size_t typeIndex) const noexcept;
		[[nodiscard]] bool isUnionType(std::size_t typeIndex) const noexcept;
		[[nodiscard]] bool isEnumType(std::size_t typeIndex) const noexcept;
		[[nodiscard]] bool isBitFieldType(std::size_t typeIndex) const noexcept;
		[[nodiscard]] bool isArrayType(std::size_t typeIndex) const noexcept;
	
		[[nodiscard]] static msf::UInt32 resolvePointerTypeIndex(const msf::codeview::Record *record) noexcept;
		[[nodiscard]] msf::UnderlyingType underlyingType(std::size_t typeIndex) const noexcept;
	
		using TypeVariant = std::variant<msf::types::Variable, msf::types::Enumerator>;
		[[nodiscard]] std::vector<TypeVariant> parseFieldList(std::size_t typeIndex, std::size_t expectedNumberOfFields) const;
		[[nodiscard]] std::vector<msf::types::Variable> parseVariables(std::size_t typeIndex, std::size_t expectedNumberOfFields) const;
		[[nodiscard]] std::vector<msf::types::Enumerator> parseEnumerators(std::size_t typeIndex, std::size_t expectedNumberOfFields) const;
	
		void parseClass(std::size_t typeIndex, const msf::codeview::Record *record);
		void parseUnion(std::size_t typeIndex, const msf::codeview::Record *record);
		void parseEnum(std::size_t typeIndex, const msf::codeview::Record *record);
		void parseBitField(std::size_t typeIndex, const msf::codeview::Record *record);
		void parseArray(std::size_t typeIndex, const msf::codeview::Record *record);
	};
}
