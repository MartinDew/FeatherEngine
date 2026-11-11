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

double Engine::get_current_delta_time() const {
	return _current_dt;
}

} //namespace feather
