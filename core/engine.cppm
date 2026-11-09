export module feather.core:engine;

import std;
import :window;

export namespace feather {

// Engine-wide state shared by every module. The servers and the main loop live in feather.main.
class FEATHER_CORE_API Engine {
	static Engine* _instance;

	Window _main_window;
	double _current_dt = 0.0;

public:
	Engine();
	~Engine();

	static Engine& get();

	static bool is_editor();
	static bool is_headless();

	Window& get_main_window();

	double get_current_delta_time() const;
	void set_current_delta_time(double dt);

	static constexpr double simulation_time = 1.0 / 60.0;
};

} //namespace feather
