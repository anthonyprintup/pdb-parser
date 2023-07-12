#pragma once

#include "Modules/Module.hpp"
#include "Utilities/Controller.hpp"

namespace Framework {
	namespace detail {
		struct FrameworkBase {
			[[nodiscard]] bool running() const noexcept {
				return this->_running;
			}
			void stop() noexcept {
				this->_running = false;
			}
		private:
			bool _running {true};
		};
	}
	
	template<class Base>
	struct Framework: Utilities::Controller<Modules::Module>, Base {};
	struct DefaultFramework: Utilities::Controller<Modules::Module>, detail::FrameworkBase {};
}

#ifdef FW_GLOBAL
extern std::unique_ptr<Framework::DefaultFramework> gFramework;
#endif
