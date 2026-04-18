#include "engine.h"
#include "launch_settings.h"

#include <framework/assert.h>

#include <chrono>

#include <rendering/rendering_server.h>
#include <resources/mesh.h>

namespace feather {

namespace {

constexpr double simulation_time = 1.0 / 60.0;

} //namespace

Engine* Engine::_instance = nullptr;

Engine::Engine() {
	// Todo replace sdl assert by custom one
	fassert(!_instance);

	_instance = this;
	_rendering_server.init();
}

Engine::~Engine() {
	// unregister_modules();
}

bool Engine::run() {
	auto current_time = start_time;

	bool keep_running = true;

	// initialization

	// Debug stuff
	if (LaunchSettings::get().dump_db.Get()) {
		ClassDB::get()->print_db();
		return true;
	}

	_world_sim.init();

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
			_world_sim.fixed_update(simulation_time);
		}

		_current_dt = frame_time;
		_world_sim.update(frame_time);

		// Tell the renderer to render here
		// _rendering_server.set_render_capture(sim.generate_render_capture());
		_rendering_server.update(frame_time);
	}

	return true;
}

double Engine::get_current_delta_time() const {
	return _current_dt;
}

bool Engine::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}
} //namespace feather