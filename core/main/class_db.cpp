#include "class_db.h"
#include "framework/reflected.h"
#include <algorithm>
#include <memory>
#include <ranges>

#include <print>

namespace feather {

std::unique_ptr<ClassDB> ClassDB::_instance = nullptr;

ClassDB& ClassDB::get() {
	if (!_instance) {
		_instance.reset(new ClassDB());
	}
	return *_instance;
}

Reflected* ClassDB::create_object_unsafe(std::string_view name) {
	auto object_info_it = _instance->_class_infos.find(name);
	if (object_info_it != _instance->_class_infos.end()) {
		return object_info_it->second.object_create_func().get<Reflected*>().value();
	}

	return {};
}

void ClassDB::print_db() {
#ifdef BETA
	for (auto& [name, info] : _class_infos) {
		std::print("Class: {}", info.name.str());
		for (auto& prop : info.properties) {
			std::print("\tProperty: {} Type: {}", prop.name.str(), std::to_underlying<VariantType>(prop.type));
		}
	}
#endif
}

} //namespace feather