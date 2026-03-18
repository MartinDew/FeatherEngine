#include "project_settings.h"

#include "launch_settings.h"
#include <shlobj_core.h>

namespace feather {

std::unique_ptr<ProjectSettings> ProjectSettings::_instance = nullptr;

ProjectSettings::ProjectSettings() : _project_path(FileSystem::current_path()) {
	_project_path = LaunchSettings::get().project_path.Get();
}

ProjectSettings* ProjectSettings::get() {
	if (_instance)
		_instance = std::make_unique<ProjectSettings>();

	return _instance.get();
}

Path ProjectSettings::get_project_path() { return _project_path; }

static void replace_all(std::string& path, const std::string& token, const std::string& replacement) {
	size_t pos = 0;
	while ((pos = path.find(token, pos)) != std::string::npos) {
		path.replace(pos, token.length(), replacement);
		pos += replacement.length();
	}
}

Path ProjectSettings::localize_path(std::string path) {
	replace_all(path, "Res://", get_project_path().string());

#if defined(_WIN32) || defined(_WIN64)
	char sys_root[MAX_PATH];
	GetSystemWindowsDirectoryA(sys_root, MAX_PATH);
	std::string root = std::string(1, sys_root[0]) + ":\\";
#else
	std::string root = "/";
#endif
	replace_all(path, "Sys://", root);

#if defined(_WIN32) || defined(_WIN64)
	char home[MAX_PATH];
	std::string home_path =
			SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home)) ? std::string(home) + "\\" : "C:\\Users\\";
#elif defined(__APPLE__) || defined(__linux__)
	const char* home = std::getenv("HOME");
	if (!home) {
		struct passwd* pw = getpwuid(getuid());
		home = pw->pw_dir;
	}
	std::string home_path = std::string(home) + "/";
#endif
	replace_all(path, "Home://", home_path);

	return Path(path);
}

void ProjectSettings::_bind_members() {
	ClassDB::bind_property(&ProjectSettings::_project_path, "project_path", VariantType::STRING);
}

void ProjectSettings::set_project_path(Path path) { _project_path = path; }

} //namespace feather