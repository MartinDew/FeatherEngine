export module feather.core:engine;

import std;

export namespace feather {

// Engine-wide state every module can read. A concrete engine owns the servers and runs the loop.
class FEATHER_CORE_API Engine {
	static Engine* _instance;

protected:
	double _current_dt = 0.0;

	Engine();

public:
	virtual ~Engine();

	virtual bool run() = 0;

	static Engine& get();

	static bool is_editor();

	double get_current_delta_time() const;

	static constexpr double simulation_time = 1.0 / 60.0;
};

} //namespace feather
