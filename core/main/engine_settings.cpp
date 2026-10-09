module;
#include <memory>
#include <type_traits>
#include <string_view>

module feather.core;

namespace feather {

EngineSettings* EngineSettings::_instance = nullptr;

EngineSettings::EngineSettings() = default;

EngineSettings& EngineSettings::get() {
	if (!_instance) {
		_instance = new EngineSettings();
	}

	return *_instance;
}

void EngineSettings::_register_setting(std::string_view name, std::unique_ptr<ISettingStorage> storage) {
	auto& _settings = get()._settings;
	if (_settings.contains(name)) {
		return;
	}
	_settings[name] = std::move(storage);
}

bool EngineSettings::has_setting(std::string_view name) {
	return get()._settings.contains(name);
}

size_t EngineSettings::setting_count() {
	return get()._settings.size();
}

} //namespace feather
