#pragma once

#include <framework/path.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class ProjectSettings final : public Reflected {
	FCLASS(ProjectSettings, Reflected);

	friend class Engine;

	Path _project_path;

	static std::unique_ptr<ProjectSettings> _instance;
	friend std::unique_ptr<ProjectSettings> std::make_unique<ProjectSettings>();

protected:
	ProjectSettings();
	static void _bind_members();

	void set_project_path(Path path);

public:
	static ProjectSettings* get();
};

} //namespace feather
