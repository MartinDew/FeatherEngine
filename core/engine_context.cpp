module feather.core;

import std;

namespace feather {

Window* EngineContext::_main_window = nullptr;

bool EngineContext::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}

bool EngineContext::is_headless() {
	static const bool headless = [] {
		const std::string& mode = LaunchSettings::get().windowed.Get();
		if (mode == "headless")
			return true;

		fassert(mode == "windowed", std::format("Unknown window mode '{}' (expected 'windowed' or 'headless')", mode));
		return false;
	}();

	return headless;
}

Window& EngineContext::get_main_window() {
	fassert(_main_window, "no main window registered");
	return *_main_window;
}

void EngineContext::set_main_window(Window* window) {
	_main_window = window;
}

} //namespace feather
