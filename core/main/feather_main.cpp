#include "engine.h"
#include "launch_settings.h"
#include "project_settings.h"
#include "resources/resource_loader.h"

#include <framework/register_framework_types.gen.h>
#include <main/register_main_types.gen.h>
#include <math/register_math_types.gen.h>
#include <rendering/register_rendering_types.gen.h>
#include <resources/register_resources_types.gen.h>
#include <world/register_world_types.gen.h>

#include <modules/modules.gen.h>

#include "framework/shared_library.h"
#include <filesystem>
#include <iostream>

namespace feather {

struct Main {
	ClassDB _class_db;
	LaunchSettings _launch_settings;
	ProjectSettings _project_settings;
	ResourceLoader _resource_loader;

	Main(int argc, char* argv[]);
	~Main();
	static void setup_db();

private:
	SharedLibrary _project_lib;
};

Main::Main(int argc, char* argv[]) : _class_db(), _launch_settings(std::move(argc), std::move(argv)) {
	_project_settings.init();
	setup_db();

	// Load Project DLL
	std::string proj_name = _project_settings.get_project_name();
	if (!proj_name.empty()) {
		std::vector<std::string> configs = { "Debug", "Development", "Release" };
		bool loaded = false;
		for (const auto& c : configs) {
#if defined(_WIN32) || defined(_WIN64)
			std::string prefix = "";
			std::string ext = ".dll";
#elif defined(__APPLE__)
			std::string prefix = "lib";
			std::string ext = ".dylib";
#else
			std::string prefix = "lib";
			std::string ext = ".so";
#endif
			auto path = _project_settings.get_project_path() / "bin" / c / (prefix + proj_name + ext);
			if (std::filesystem::exists(path)) {
				if (_project_lib.load(path.string())) {
					std::cout << "Loaded project DLL: " << path.string() << std::endl;
					loaded = true;
					break;
				}
			}
		}

		if (loaded) {
			Callable reg_func = _project_lib.get_symbol("register_project_types");
			if (reg_func.is_valid()) {
				reg_func.call();
				std::cout << "Project types registered successfully." << std::endl;
			}
			else {
				std::cerr << "Failed to find 'register_project_types' in project DLL." << std::endl;
			}
		}
	}

	Engine engine;

	engine.run();

	unregister_modules();
}

Main::~Main() {
	_project_lib.unload();
}

void Main::setup_db() {
	register_framework_types();
	register_math_types();
	register_resources_types();
	register_rendering_types();
	register_world_types();
	register_main_types();

	// then register module types
	register_modules();
}

} //namespace feather

int main(int argc, char* argv[]) {
	feather::Main fmain(std::move(argc), std::move(argv));
}