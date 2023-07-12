#include "Structure.hpp"

using namespace msf::types;

bool Properties::uniqueName() const noexcept {
	constexpr UInt16 hasUniqueName {0x0200};
	return (this->properties & hasUniqueName) != 0;
}

bool Properties::forwardReference() const noexcept {
	constexpr UInt16 isForwardReference {0x0080};
	return (this->properties & isForwardReference) != 0;
}

const Variable *Structure::variable(const HashType hash) const noexcept {
	for (const auto &variable : this->variables)
		if (variable.name == hash)
			return &variable;
	return nullptr;
}
