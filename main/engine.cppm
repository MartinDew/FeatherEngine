export module feather.main:engine;

import std;
import feather.core;
import feather.world;
import feather.rendering;

export namespace feather {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct Main;

class FEATHER_MAIN_API Engine {
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

	inline static Engine& get() { return *_instance; }

	inline Window& get_main_window() { return _main_window; }

	double get_current_delta_time() const;

	static constexpr double simulation_time = EngineContext::simulation_time;
};

} //namespace feather
