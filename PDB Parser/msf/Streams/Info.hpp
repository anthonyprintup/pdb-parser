#pragma once

#include "Stream.hpp"

namespace msf::streams {
	struct Info: Stream {
		explicit Info(std::unique_ptr<MappedBlockStream> stream) noexcept;

		[[nodiscard]] bool reload();

	private:
		struct NamedStreamsMap {
			std::vector<std::string_view> streams {};

			[[nodiscard]] bool read(stream::Reader &reader);
		};

		const InfoStreamHeader *header {};
		NamedStreamsMap namedStreams {};
		UInt32 features {};
	};
}
