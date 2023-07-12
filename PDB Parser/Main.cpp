// ReSharper disable CppClangTidyClangDiagnosticUnusedPrivateField
// ReSharper disable CppClangTidyClangDiagnosticShadowField
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <span>
#include <array>
#include <variant>
#include <bitset>
#include <ranges>
#include <unordered_map>

#include <fmt/format.h>

#include <Framework/Utilities/Strings/Fnv1A.hpp>

#include "Cache.hpp"
using Hash = Framework::Utilities::Strings::Fnv1A::Hash;

#include "Data Stream/Reader.hpp"
using namespace stream;

#include "msf/Type Index.hpp"
#include "msf/Leaf Type.hpp"
#include "msf/Symbol Type.hpp"
using namespace msf;
#include "msf/Types/Public Symbol.hpp"
#include "msf/Types/Structure.hpp"
#include "msf/Types/Union.hpp"
#include "msf/Types/Enum.hpp"
#include "msf/Types/Bit Field.hpp"
#include "msf/Types/Array.hpp"
using namespace types;

using FileStreamType      = std::basic_string<char>;
using ByteInputStreamType = std::basic_ifstream<char>;
FileStreamType readFile(const std::filesystem::path &path, const std::uintmax_t size = 0) {
	const auto fileSize = std::filesystem::file_size(path);
	if (!fileSize || fileSize < size)
		return {};
	
	ByteInputStreamType stream {path, std::ios::binary};
	if (!stream)
		return {};
	
	FileStreamType buffer {};
	buffer.resize(!size ? fileSize : size);

	if (!stream.read(buffer.data(), buffer.size()))
		return {};

	return buffer;
}

#include "msf/Pdb.hpp"
#include "Helpers.hpp"

// Thanks to the LLVM team!
namespace pdb {
	std::optional<const Pdb> load(const std::filesystem::path &path) {
		const auto bytes = readFile(path);
		if (bytes.empty())
			return std::nullopt;

		const SpanType span {reinterpret_cast<SpanType::element_type*>(bytes.data()), bytes.length()};
		return msf::load(span);
	}
}

int main() {
	const auto pdb = pdb::load("ntkrnlmp.pdb");
	if (!pdb) return 0;

	// Symbol Records
	pdb->symbolRecords()->iterate([&](const PublicSymbol32 &symbol) {
		const auto &sectionHeaders = pdb->debugInformationStream()->sectionHeaders();
		const auto sectionOffset = symbol.segment >= sectionHeaders.size() ? 0 : sectionHeaders[symbol.segment - 1].virtualAddress;

		const auto symbolName = symbol.name();
		if (symbolName.find("Debug") != std::string_view::npos)
			fmt::print(FMT_STRING("Name: {} | Offset: 0x{:X}\n"), symbolName, sectionOffset + symbol.offset);
		fmt::print(FMT_STRING("Name: {}\n"), symbolName);
	});

	pdb::Cache typeCache {pdb.value()};
	const auto eProcess = typeCache.structure(Fnv1A("_EPROCESS"));
	if (!eProcess)
		return 0;

	fmt::print(FMT_STRING("_EPROCESS\n  size: 0x{:X}\n  variables: {}\n"), eProcess->size, eProcess->variables.size());

	const auto pcb = eProcess->variable(Fnv1A("Pcb"));
	if (!pcb || pcb->type != UnderlyingType::STRUCTURE)
		return 0;

	const auto kProcess = typeCache.structure(pcb);
	if (!kProcess)
		return 0;

	fmt::print(FMT_STRING("_KPROCESS\n  size: 0x{:X}\n  _EPROCESS->Pcb: 0x{:X}\n"), kProcess->size, pcb->offset);

	const auto directoryTableBase = kProcess->variable(Fnv1A("DirectoryTableBase"));
	if (!directoryTableBase || directoryTableBase->type != UnderlyingType::ULONG64)
		return 0;
	
	fmt::print(FMT_STRING("DirectoryTableBase\n  _KPROCESS->DirectoryTableBase: 0x{:X}"), directoryTableBase->offset);
}