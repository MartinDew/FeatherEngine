#include "engine.h"
#include "launch_settings.h"

#include <framework/assert.hpp>

#include <chrono>

// temp. Need a way to have dynamic associations for object types
#include "modules/vex_renderer/vex_renderer.h"
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
	_rendering_server.use_renderer<VexRenderer>();
}

bool Engine::run() {
	auto current_time = start_time;

	bool keep_running = true;

	// initialization

	// update
	double accumulator = 0.0;
	while (keep_running) {
		keep_running = _main_window.update();

		auto new_time = Clock::now();

		double frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - current_time).count();
		current_time = new_time;

		accumulator += frame_time;
		while (accumulator >= simulation_time) {
			accumulator -= simulation_time;
			// _physics_update here
		}

		// Update here

		// Tell the renderer to render here
		_rendering_server.update(frame_time);
	}

	return true;
}

} //namespace feather