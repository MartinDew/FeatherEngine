#include "project_settings.h"

namespace feather {

std::unique_ptr<ProjectSettings> ProjectSettings::_instance = nullptr;

ProjectSettings::ProjectSettings() : _project_path(FileSystem::current_path()) {}

ProjectSettings* ProjectSettings::get() {
	if (_instance)
		_instance = std::make_unique<ProjectSettings>();

	return _instance.get();
}

void ProjectSettings::_bind_members() {
	ClassDB::bind_property(&ProjectSettings::_project_path, "project_path", VariantType::STRING);
}

void ProjectSettings::set_project_path(Path path) { _project_path = path; }

} //namespace feather