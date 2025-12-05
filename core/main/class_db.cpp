#include "class_db.h"
#include "framework/reflected.h"
#include <algorithm>
#include <memory>
#include <ranges>

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
		return std::get<Reflected*>(object_info_it->second.object_create_func());
	}

	return {};
}

} //namespace feather