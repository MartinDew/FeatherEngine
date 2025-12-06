#pragma once

#include <args.hxx>
#include <filesystem>

namespace feather {

class LaunchSettings {
	args::ArgumentParser _parser{ "Feather Engine" };

	static LaunchSettings* instance;

public:
	LaunchSettings(int argc, char* argv[]);

	args::Positional<std::filesystem::path> project_path{ _parser, "project path", "The path to the project directory", std::filesystem::current_path().c_str() };

#ifdef EDITOR_BUILD
	// Editor mode?
	args::ImplicitValueFlag<bool> editor_mode{ _parser, "editor", "Should run in editor mode", { 'e', "editor" }, true, false };
#endif

	// will be more complex eventually but for now just a flag for windowed vs dummy
	args::ValueFlag<std::string> windowed{ _parser, "window mode", "The window mode to use (windowed {default} | headless )", { "w" }, "windowed" };

	static LaunchSettings& get() { return *instance; }

private:
	args::HelpFlag _help{ _parser, "help", "Display this help menu", { 'h', "help" } };
};

} //namespace feather