#pragma once

#include <framework/path.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class ProjectSettings final : public Reflected {
	FCLASS_SINGLETON(ProjectSettings, Reflected);

	friend class Engine;

	Path _project_path;
	std::string _project_name;

protected:
	static void _bind_members();

	void set_project_path(Path path);

public:
	ProjectSettings();
	bool init();

	Path get_project_path();
	std::string get_project_name() const;
	Path localize_path(const Path& path);
};

} //namespace feather
