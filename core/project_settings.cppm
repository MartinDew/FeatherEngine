module;

#include <framework/reflection_macros.h>

#include <framework/singleton_helpers.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "project_settings.gen.h"
#endif

export module feather.core:project_settings;

import std;
import feather.framework;
import feather.reflection;

export namespace feather {

class FEATHER_CORE_API ProjectSettings final : public Reflected {
	FCLASS(singleton);

	[[get(public), set(public)]]
	Path _project_path;
	std::string _project_name;

public:
	ProjectSettings();
	bool init();

	std::string get_project_name() const;
	Path localize_path(const Path& path);
};

} //namespace feather
