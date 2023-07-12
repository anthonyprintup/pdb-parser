#include "Layouts.hpp"

#include "../../Helpers.hpp"

using namespace msf;

std::unique_ptr<streams::MappedBlockStream> streams::createStream(UInt32 blockSize, const MsfStreamLayout &layout) {
	return std::make_unique<MappedBlockStream>(blockSize, layout);
}

std::unique_ptr<streams::MappedBlockStream> streams::createIndexedStream(const MsfLayout &msf, const UInt32 streamIndex) {
	if (streamIndex >= msf.streamMap.size())
		return nullptr;

	MsfStreamLayout layout {.length = msf.streamSizes[streamIndex], .blocks = msf.streamMap[streamIndex]};
	return std::make_unique<MappedBlockStream>(msf.superBlock->blockSize, layout);
}

UInt32 streams::getFpmIntervals(const MsfLayout &msf) noexcept {
	return static_cast<UInt32>(pdb::divideCeil(msf.superBlock->numberOfBlocks, 8ull * msf.superBlock->blockSize));
}

UInt32 streams::getFpmInternalLength(const MsfLayout &msf) noexcept {
	return msf.superBlock->blockSize;
}

streams::MsfStreamLayout streams::fpmStreamLayout(const MsfLayout &msf, const bool alternateFpm) {
	const auto fpmBlockIntervals = getFpmIntervals(msf);

	MsfStreamLayout layout {};
	for (UInt32 i {}, fpmBlockNumber = alternateFpm ? msf.alternateFpmBlock() : msf.mainFpmBlock();
		i < fpmBlockIntervals; ++i) {
		layout.blocks.emplace_back(fpmBlockNumber);
		fpmBlockNumber += getFpmInternalLength(msf);
	}
	layout.length = pdb::divideCeil(msf.superBlock->numberOfBlocks, 8);
	return layout;
}

std::unique_ptr<streams::MappedBlockStream> streams::fpmStream(const MsfLayout &msf, const stream::SpanType bytes) {
	auto fpmStream = createStream(msf.superBlock->blockSize, fpmStreamLayout(msf));
	fpmStream->readBlocks(msf, bytes);

	return fpmStream;
}

std::unique_ptr<streams::MappedBlockStream> streams::directoryStream(const MsfLayout &msf, const stream::SpanType bytes) {
	const MsfStreamLayout layout {.length = msf.superBlock->numberOfDirectoryBytes, .blocks = msf.directoryBlocks};
	auto directoryStream = createStream(msf.superBlock->blockSize, layout);
	directoryStream->readBlocks(msf, bytes);

	return directoryStream;
}
