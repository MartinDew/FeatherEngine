export module feather.main:default_engine;

import std;
import feather.core;
import feather.world;
import feather.rendering;

export namespace feather {

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

extern "C++" struct Main;

class DefaultEngine final : public Engine {
	friend Main;

	DisplayServer _display_server;
	RenderingServer _rendering_server;
	WorldSim _world_sim;

	// the time accumulator
	TimePoint start_time = Clock::now();

	DefaultEngine();

public:
	~DefaultEngine() override;

	bool run() override;
};

} //namespace feather
