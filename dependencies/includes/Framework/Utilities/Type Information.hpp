#pragma once

#include "Strings/Fnv1A.hpp"

namespace internal {
	template<class T>
	struct Type {
		static constexpr auto hash() { return Fnv1A(__FUNCTION__); }
		
		#ifdef BUILD_DEBUG
		static constexpr const char *name() { return __FUNCTION__; }
		#endif
	};
}