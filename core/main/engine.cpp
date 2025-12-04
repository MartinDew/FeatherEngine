#include "engine.h"
#include "launch_settings.h"
#include "modules/modules.gen.h"

#include <framework/assert.h>

#include <chrono>

#include "rendering/rendering_server.h"

namespace feather {

namespace {

constexpr double simulation_time = 1.0 / 60.0;

} //namespace

Engine* Engine::_instance = nullptr;

Engine::Engine() {
	// Todo replace sdl assert by custom one
	fassert(!_instance);

	_instance = this;
	// register_modules();
}

Engine::~Engine() {
	// unregister_modules();
}

bool Engine::run() {
	auto current_time = start_time;

	bool keep_running = true;

	// initialization

	// Todo, remove
	_rendering_server.use_renderer("VexRenderer"_ss);

	// update
	double accumulator = 0.0;
	while (keep_running) {
		keep_running = _main_window.update();

		auto new_time = Clock::now();

		double frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - current_time).count();
		current_time = new_time;

		accumulator += frame_time;
		_current_dt = simulation_time;
		while (accumulator >= simulation_time) {
			accumulator -= simulation_time;
			// _physics_update here
		}

		_current_dt = frame_time;
		// Update here

		// Tell the renderer to render here
		_rendering_server.update(frame_time);
	}

	return true;
}

double Engine::get_current_delta_time() const { return _current_dt; }

bool Engine::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}
} //namespace feather