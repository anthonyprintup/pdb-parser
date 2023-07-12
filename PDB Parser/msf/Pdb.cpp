#include "Pdb.hpp"

#include "Streams/Layouts.hpp"
#include "../Helpers.hpp"

using namespace msf;

Pdb::Pdb(const SuperBlock *superBlock, stream::SpanType bytes):
	msfLayout {.superBlock = superBlock}, bytes {bytes} {
	this->parseFileHeaders();
	this->parseStreamData();
}

const streams::Info *Pdb::infoStream() {
	if (this->info)
		return this->info.get();

	auto indexedStream = createIndexedStream(this->msfLayout, static_cast<UInt32>(StreamIndex::PDB));
	if (!indexedStream)
		return nullptr;

	indexedStream->readBlocks(this->msfLayout, this->bytes);
	this->info = std::make_unique<streams::Info>(std::move(indexedStream));
	if (!this->info->reload()) {
		this->info.reset();
		return nullptr;
	}

	return this->info.get();
}

const streams::Info *Pdb::infoStream() const noexcept {
	return this->info.get();
}

const streams::Tpi *Pdb::tpiStream() {
	if (this->tpi)
		return this->tpi.get();

	auto indexedStream = createIndexedStream(this->msfLayout, static_cast<UInt32>(StreamIndex::TPI));
	if (!indexedStream)
		return nullptr;

	indexedStream->readBlocks(this->msfLayout, this->bytes);
	this->tpi = std::make_unique<streams::Tpi>(std::move(indexedStream));
	if (!this->tpi->reload()) {
		this->tpi.reset();
		return nullptr;
	}

	return this->tpi.get();
}

const streams::Tpi *Pdb::tpiStream() const noexcept {
	return this->tpi.get();
}

std::pair<std::size_t, std::size_t> Pdb::typeIndexRange() const noexcept {
	if (!this->tpi)
		return {};
	return this->tpi->typeIndexRange();
}

const streams::Dbi *Pdb::debugInformationStream() {
	if (this->dbi)
		return this->dbi.get();

	auto indexedStream = createIndexedStream(this->msfLayout, static_cast<UInt32>(StreamIndex::DBI));
	if (!indexedStream)
		return nullptr;

	indexedStream->readBlocks(this->msfLayout, this->bytes);
	this->dbi = std::make_unique<streams::Dbi>(std::move(indexedStream));
	if (!this->dbi->reload(this->msfLayout, this->bytes)) {
		this->dbi.reset();
		return nullptr;
	}

	return this->dbi.get();
}

const streams::Dbi *Pdb::debugInformationStream() const noexcept {
	return this->dbi.get();
}

const streams::SymbolRecords *Pdb::symbolRecords() {
	if (this->symbols)
		return this->symbols.get();
	if (!this->dbi)
		return nullptr;

	auto indexedStream = createIndexedStream(this->msfLayout, this->dbi->symbolRecordStreamIndex());
	if (!indexedStream)
		return nullptr;

	indexedStream->readBlocks(this->msfLayout, this->bytes);
	this->symbols = std::make_unique<streams::SymbolRecords>(std::move(indexedStream));
	return this->symbols.get();
}

const streams::SymbolRecords *Pdb::symbolRecords() const noexcept {
	return this->symbols.get();
}

void Pdb::parseFileHeaders() {
	// Initialize the Free Page Map
	this->msfLayout.freePageMap.resize(this->msfLayout.superBlock->numberOfBlocks);

	const auto fpmStream = streams::fpmStream(this->msfLayout, this->bytes);
	for (UInt32 blocksRemaining = this->msfLayout.superBlock->numberOfBlocks, blockIndex {};
		auto &&byte : fpmStream->bytes) {
		const auto blocksThisByte = std::min(blocksRemaining, 8u);
		for (std::size_t i {}; i < blocksThisByte; ++i) {
			if (byte & 1 << i)
				this->msfLayout.freePageMap[blockIndex] = true;
			--blocksRemaining;
			++blockIndex;
		}
	}

	const auto blockMapOffset = this->msfLayout.blockMapOffset();
	const auto numberOfDirectoryBlocks = this->msfLayout.numberOfDirectoryBlocks();
	const auto blockMap = reinterpret_cast<std::uintptr_t>(this->msfLayout.superBlock) + blockMapOffset;
	for (std::size_t i {}; i < numberOfDirectoryBlocks; ++i) {
		const auto blockNumber = reinterpret_cast<UInt32*>(blockMap)[i];
		this->msfLayout.directoryBlocks.emplace_back(blockNumber);
	}
}

void Pdb::parseStreamData() {
	auto directoryStream = streams::directoryStream(this->msfLayout, this->bytes);
	const auto directoryStreamData = reinterpret_cast<const UInt32*>(directoryStream->bytes.data());
	const auto numberOfStreams = directoryStreamData[0];
	auto blockArray = directoryStreamData + numberOfStreams + 1;
	for (std::size_t i {}; i < numberOfStreams; ++i) {
		const auto streamSize = directoryStreamData[i + 1];
		this->msfLayout.streamSizes.emplace_back(streamSize);

		const auto expectedNumberOfStreamBlocks = streamSize == UINT32_MAX ? 0 : pdb::bytesToBlocks(streamSize, this->msfLayout.superBlock->blockSize);
		auto &stream = this->msfLayout.streamMap.emplace_back();
		for (std::size_t j {}; j < expectedNumberOfStreamBlocks; ++j, ++blockArray) {
			const auto blockNumber = blockArray[0];
			stream.emplace_back(blockNumber);
		}
	}

	this->directoryStream = std::move(directoryStream);
}

std::size_t Pdb::numberOfStreams() const noexcept {
	return this->msfLayout.streamSizes.size();
}

std::optional<const Pdb> msf::load(const stream::SpanType bytes) {
	if (const auto sizeInBytes = bytes.size_bytes();
		sizeInBytes < sizeof(SuperBlock))
		return std::nullopt;
	if (!pdb::verifyMagic(bytes))
		return std::nullopt;

	const auto superBlock = reinterpret_cast<const SuperBlock*>(bytes.data());
	if (!pdb::verifySuperBlock(*superBlock))
		return std::nullopt;
	if (bytes.size_bytes() % superBlock->blockSize != 0) // File size is not a multiple of block size
		return std::nullopt;

	Pdb pdb {superBlock, bytes};

	// Parse the PDB Info Stream
	if (!pdb.infoStream())
		return std::nullopt;

	// Parse the Type Manager Stream
	if (!pdb.tpiStream())
		return std::nullopt;

	// Parse the Debug Information Stream
	if (!pdb.debugInformationStream())
		return std::nullopt;

	// Parse the Symbol Records stream
	if (!pdb.symbolRecords())
		return std::nullopt;

	return pdb;
}

std::vector<const codeview::Record*> msf::typeRecords(const Pdb &pdb) {
	std::vector<const codeview::Record*> records {};
	pdb.tpiStream()->iterate([&](decltype(records)::value_type record) {
		records.emplace_back(record);
	});

	return records;
}
