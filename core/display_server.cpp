module feather.core;

import std;

namespace feather {

DisplayServer* DisplayServer::_instance = nullptr;

DisplayServer::DisplayServer() {
	fassert(!_instance);
	_instance = this;
}

DisplayServer::~DisplayServer() {
	_instance = nullptr;
}

DisplayServer& DisplayServer::get() {
	return *_instance;
}

bool DisplayServer::is_headless() {
	static const bool headless = [] {
		const std::string& mode = LaunchSettings::get().windowed.Get();
		if (mode == "headless")
			return true;

		fassert(mode == "windowed", std::format("Unknown window mode '{}' (expected 'windowed' or 'headless')", mode));
		return false;
	}();

	return headless;
}

Window& DisplayServer::get_main_window() {
	return _main_window;
}

} //namespace feather
