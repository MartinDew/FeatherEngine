module feather.main;

import std;
import feather.modules;

namespace feather {

// Global module: ClassDB befriends this Main from feather.reflection (see class_db.cppm).
extern "C++" {

struct Main {
	ClassDB _class_db;
	LaunchSettings _launch_settings;
	ProjectSettings _project_settings;
	ResourceLoader _resource_loader;

	Main(int argc, char* argv[]);
	~Main();
	void setup_db();
};

Main::Main(int argc, char* argv[]) : _class_db(), _launch_settings(std::move(argc), std::move(argv)) {
	if (!_project_settings.init() && !_launch_settings.demo_mode.Get())
		return;

	setup_db();

	DefaultEngine engine;

	engine.run();

	unregister_modules();
}

Main::~Main() {
	ClassDB::clear();
}

void Main::setup_db() {
	register_framework_types();
	register_math_types();
	register_reflection_types();
	register_core_types();
	register_resources_types();
	register_rendering_types();
	register_world_types();

	// then register module types
	register_modules();
}

} // extern "C++"

int run(int argc, char* argv[]) {
	Main fmain(argc, argv);
	return 0;
}

} //namespace feather
