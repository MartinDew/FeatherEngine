export module feather.core:engine_context;

import :window;

export namespace feather {

// Engine-wide state that modules below main read without depending on Engine.
class FEATHER_CORE_API EngineContext {
	static Window* _main_window;

public:
	static bool is_editor();
	static bool is_headless();

	static Window& get_main_window();
	static void set_main_window(Window* window);

	static constexpr double simulation_time = 1.0 / 60.0;
};

} //namespace feather
