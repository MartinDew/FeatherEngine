module feather.core;

import std;

namespace feather {

Engine* Engine::_instance = nullptr;

Engine::Engine() {
	fassert(!_instance);
	_instance = this;
}

Engine::~Engine() {
	_instance = nullptr;
}

Engine& Engine::get() {
	return *_instance;
}

bool Engine::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}

bool Engine::is_headless() {
	static const bool headless = [] {
		const std::string& mode = LaunchSettings::get().windowed.Get();
		if (mode == "headless")
			return true;

		fassert(mode == "windowed", std::format("Unknown window mode '{}' (expected 'windowed' or 'headless')", mode));
		return false;
	}();

	return headless;
}

Window& Engine::get_main_window() {
	return _main_window;
}

double Engine::get_current_delta_time() const {
	return _current_dt;
}

void Engine::set_current_delta_time(double dt) {
	_current_dt = dt;
}

} //namespace feather
