export module feather.core:display_server;

import std;
import :window;

export namespace feather {

// Owns the main window. Created before anything that renders.
class FEATHER_CORE_API DisplayServer {
	static DisplayServer* _instance;

	Window _main_window;

public:
	DisplayServer();
	~DisplayServer();

	static DisplayServer& get();

	// Headless has no window events; selected by the launch settings.
	static bool is_headless();

	Window& get_main_window();
};

} //namespace feather
