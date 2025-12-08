#pragma once

#include "framework/assert.h"
#include "framework/static_string.hpp"

#include <args.hxx>

#include <filesystem>
#include <map>

namespace feather {

class LaunchSettings {
	args::ArgumentParser _parser { "Feather Engine" };

	static LaunchSettings* _instance;

	LaunchSettings();
	//
	// size_t argc = 0;
	// char** argv;

	std::map<StaticString, args::Group*> _groups { { "root"_ss, &_parser } };

public:
	LaunchSettings(int argc, char* argv[]);
	void init(int argc, char* argv[]);

	args::Positional<std::filesystem::path> project_path { _parser, "project path", "The path to the project directory",
		std::filesystem::current_path().c_str() };

#ifdef EDITOR_BUILD
	// Editor mode?
	args::ImplicitValueFlag<bool> editor_mode { _parser, "editor", "Should run in editor mode", { 'e', "editor" }, true,
		false };
#endif

	// will be more complex eventually but for now just a flag for windowed vs dummy
	args::ValueFlag<std::string> windowed { _parser, "window mode",
		"The window mode to use (windowed {default} | headless )", { "w" }, "windowed" };

	args::ValueFlag<std::string> renderer;

#ifdef EDITOR_BUILD
	args::ImplicitValueFlag<bool> dump_db { _parser, "dump db", "dumps the class database", { "dump-db" }, true,
		false };
#endif

	static LaunchSettings& get();

	static constexpr args::Group& get_group(StaticString name = "root"_ss) {
		if (name == "root"_ss) {
			return get()._parser;
		}

		fassert(get()._groups.contains(name), std::format("No such group {}", name));
		return *get()._groups.at(name);
	}

private:
	args::HelpFlag _help { _parser, "help", "Display this help menu", { 'h', "help" } };
};

} //namespace feather