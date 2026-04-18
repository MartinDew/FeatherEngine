#pragma once

#include "launch_settings.h"
#include "rendering/rendering_server.h"
#include "window.h"
#include "world_sim.h"

#include <chrono>

namespace feather {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class Engine {
	friend Main;
	static Engine* _instance;

	RenderingServer _rendering_server;
	Window _main_window;
	WorldSim _world_sim;

	// the time accumulator
	TimePoint start_time = Clock::now();
	double _current_dt = 0.0;

	Engine();

public:
	~Engine();

	bool run();

	static Engine& get() { return *_instance; }

	static bool is_editor();

	Window& get_main_window() { return _main_window; }

	double get_current_delta_time() const;
};

} //namespace feather
