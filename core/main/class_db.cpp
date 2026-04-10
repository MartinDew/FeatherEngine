#include "class_db.h"
#include "framework/reflected.h"
#include <algorithm>
#include <memory>
#include <ranges>

#include <print>

namespace feather {

FSINGLETON_INSTANCE(ClassDB);

ClassDB::ClassDB() {
	FSINGLETON_CONSTRUCT_INSTANCE()
	_class_infos.insert(std::make_pair("Reflected", ClassInfo { .name = "Reflected"_ss, .parent = ""_ss }));
}

Reflected* ClassDB::create_object_unsafe(std::string_view name) {
	auto object_info_it = _instance->_class_infos.find(name);
	if (object_info_it != _instance->_class_infos.end()) {
		return object_info_it->second.object_create_func().as<Reflected*>().value();
	}

	return {};
}

std::vector<StaticString> ClassDB::_get_children_names_internal(const ClassInfo& object, bool exclusive) {
	std::vector<StaticString> children;
	children.reserve(object.children.size());
	for (auto& child : object.children) {
		children.push_back(child->name);
	}

	for (auto& child : object.children) {
		auto sub_children = _get_children_names_internal(*child, exclusive);
		children.append_range(sub_children);
	}
	return children;
}

ClassInfo* ClassDB::_get_class_info_internal(std::string_view name) {
	if (auto it = get()->_class_infos.find(name); it != get()->_class_infos.end()) {
		return &it->second;
	};
	return nullptr;
}

std::vector<StaticString> ClassDB::get_children_names(std::string_view object_name, bool exclusive) {
	if (auto it = ClassDB::get()->_class_infos.find(object_name); it != ClassDB::get()->_class_infos.end()) {
		return _get_children_names_internal(it->second, exclusive);
	}

	return {};
}

std::string ClassDB::get_children_names_string(StaticString object_name, bool exclusive) {
	std::string children_str;
	auto children = get_children_names(object_name, exclusive);
	for (auto& child : children) {
		children_str += child.str();
		children_str += " ";
	}
	return children_str;
}

void ClassDB::print_db() {
#ifdef BETA
	std::println("Printing database");

	for (auto& [name, info] : _class_infos) {
		std::println("Class: {} : ", info.name.str());
		for (auto& prop : info.properties) {
			std::println("\tProperty: {} Type: {}\n", prop.name.str(), std::to_underlying<VariantType>(prop.type));
		}

		std::println("Children : {}", get_children_names_string(name, false));
	}
#endif
}

} //namespace feather