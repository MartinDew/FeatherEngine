#pragma once

#include "launch_settings.h"
#include "rendering/rendering_server.h"
#include "window.h"

#include <chrono>

namespace feather {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

class Engine {
	Window _main_window;

	// the time accumulator
	TimePoint start_time = Clock::now();

	static Engine* _instance;

	RenderingServer _rendering_server;

	double _current_dt = 0.0;

public:
	Engine();
	~Engine();

	bool run();

	static Engine& get() { return *_instance; }

	static bool is_editor();

	Window& get_main_window() { return _main_window; }

	double get_current_delta_time() const;
};

} //namespace feather
