#pragma once

namespace Framework::Events {
	enum struct Priority {
		HIGHEST,
		HIGH,
		MEDIUM,
		LOW,
		LOWEST,
		DEFAULT = MEDIUM
	};
}
