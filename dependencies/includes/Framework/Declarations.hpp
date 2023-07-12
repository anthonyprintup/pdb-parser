#pragma once

namespace Framework::Utilities::Strings::Fnv1A {
	struct Hash;
}
using Hash = Framework::Utilities::Strings::Fnv1A::Hash;

namespace Framework::Modules {
	struct Module;
}
using Module = Framework::Modules::Module;

namespace Framework::Utilities {
	template<class T>
	struct Controller;
	template<class T>
	struct ConcurrentController;
	struct MsTimer;
}
template<class T>
using Controller = Framework::Utilities::Controller<T>;
template<class T>
using ConcurrentController = Framework::Utilities::ConcurrentController<T>;
using Timer = Framework::Utilities::MsTimer;

namespace Framework::Events {
	struct Event;
	enum struct Priority;
	template<class T>
	struct Listener;
	template<class T>
	struct ConcurrentListener;
}

using Event = Framework::Events::Event;
using EventPriority = Framework::Events::Priority;

template<class E>
using EventListener = Framework::Events::Listener<E>;
template<class E>
using ConcurrentEventListener = Framework::Events::ConcurrentListener<E>;

// ReSharper disable once CppInconsistentNaming
namespace Framework::Utilities::RandomNumberGenerators {}
namespace rng = Framework::Utilities::RandomNumberGenerators;  // NOLINT(misc-unused-alias-decls)

namespace Framework::Utilities::Hooks {
	struct VirtualFunctionTable;
}

using VirtualFunctionTable = Framework::Utilities::Hooks::VirtualFunctionTable;
