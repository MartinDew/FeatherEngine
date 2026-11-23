module;

#include <framework/singleton_helpers.h>

module feather.reflection;

import std;

namespace feather {

FSINGLETON_INSTANCE(ClassDB);

ClassDB::ClassDB() {
	FSINGLETON_CONSTRUCT_INSTANCE()
	_register<Reflected>(ClassKind::Abstract);
}

ClassDB::~ClassDB() {
	clear();
}

void ClassDB::clear() {
	_instance->_class_infos.clear();
	_instance->_enum_infos.clear();
	_instance->_subclass_delegates.clear();
	_instance->_class_registered_delegate.clear();
}

Reflected* ClassDB::instantiate(StaticString class_name) {
	const ClassInfo* info = get_class_info(class_name);
	return info && info->create_reflected ? info->create_reflected() : nullptr;
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

ClassInfo* ClassDB::_get_class_info_internal(StaticString name) {
	if (auto it = get()->_class_infos.find(name); it != get()->_class_infos.end()) {
		return &it->second;
	};
	return nullptr;
}

const EnumInfo* ClassDB::get_enum(StaticString name) {
	auto& infos = get()->_enum_infos;
	auto it = infos.find(name);
	return it == infos.end() ? nullptr : &it->second;
}

std::vector<StaticString> ClassDB::get_children_names(StaticString object_name, bool exclusive) {
	if (const ClassInfo* info = _get_class_info_internal(object_name)) {
		return _get_children_names_internal(*info, exclusive);
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

Delegate<std::string_view>::id_t ClassDB::on_subclass_registered(
		std::string_view base_class_name,
		const Delegate<std::string_view>::DelegateFuncType& callback
) {
	return get()->_subclass_delegates[StaticString(base_class_name)].subscribe(callback);
}

void ClassDB::unregister_subclass_delegate(std::string_view base_class_name, Delegate<std::string_view>::id_t id) {
	auto it = get()->_subclass_delegates.find(StaticString(base_class_name));
	if (it != get()->_subclass_delegates.end()) {
		it->second.remove(id);
	}
}

Delegate<std::string_view>::id_t
ClassDB::on_class_registered(const Delegate<std::string_view>::DelegateFuncType& callback) {
	return get()->_class_registered_delegate.subscribe(callback);
}

void ClassDB::unregister_class_delegate(Delegate<std::string_view>::id_t id) {
	get()->_class_registered_delegate.remove(id);
}

std::vector<StaticString> ClassDB::get_class_names() {
	std::vector<StaticString> names;
	names.reserve(get()->_class_infos.size());
	for (const auto& [name, info] : get()->_class_infos) {
		names.push_back(name);
	}
	return names;
}

void ClassDB::_fire_subclass_delegates(const ClassInfo& info) {
	for (const ClassInfo* parent = info.parent_info; parent; parent = parent->parent_info) {
		auto it = get()->_subclass_delegates.find(parent->name);
		if (it != get()->_subclass_delegates.end()) {
			it->second.execute(info.name);
		}
	}
}

bool ClassDB::has_parent(StaticString object_name, StaticString parent_name) {
	const ClassInfo* info = _get_class_info_internal(object_name);
	for (const ClassInfo* parent = info ? info->parent_info : nullptr; parent; parent = parent->parent_info) {
		if (parent->name == parent_name) {
			return true;
		}
	}
	return false;
}

#ifdef BETA
// TODO consider adding code directly in variant
static std::string_view variant_type_name(VariantType type) {
	static constexpr std::array<std::string_view, 19> names { "NIL",	 "BOOL",	  "INT",   "FLOAT",	  "VECTOR3",
															  "VECTOR2", "VERTEX",	  "COLOR", "VECTOR4", "QUATERNION",
															  "MATRIX",	 "TRANSFORM", "RID",   "STRING",  "ARRAY",
															  "PATH",	 "OBJECT",	  "VALUE", "INVALID" };
	size_t index = std::to_underlying(type);
	return index < names.size() ? names[index] : "UNKNOWN";
}

static std::string_view access_name(AccessLevel access) {
	switch (access) {
	case AccessLevel::Public:
		return "public";
	case AccessLevel::Protected:
		return "protected";
	case AccessLevel::Private:
		return "private";
	}
	return "unknown";
}
#endif

void ClassDB::print_db() {
#ifdef BETA
	std::println("Class database: {} classes, {} enums", _class_infos.size(), _enum_infos.size());

	std::vector<const ClassInfo*> classes;
	for (auto& [name, info] : _class_infos) {
		classes.push_back(&info);
	}
	std::ranges::sort(classes, {}, [](const ClassInfo* info) { return info->name.str(); });

	for (const ClassInfo* class_info : classes) {
		const ClassInfo& info = *class_info;
		const char* kind = info.is_singleton ? "singleton" : info.is_abstract ? "abstract" : "concrete";
		std::println("\nClass: {} [{}]", info.name.str(), kind);
		std::println("\tParent: {}", info.parent_info ? info.parent_info->name.str() : "(none)");
		std::println("\tSize: {} Alignment: {} Creatable: {}", info.size, info.alignment, info.can_create());
		std::println("\tChildren: {}", get_children_names_string(info.name, false));

		std::println("\tProperties ({}):", info.properties.size());
		for (const auto& prop : info.properties) {
			std::print(
					"\t\t{}: {} (get: {}, set: {})",
					prop.name.str(),
					variant_type_name(prop.type),
					prop.getter ? access_name(prop.getter_access) : "none",
					prop.setter ? access_name(prop.setter_access) : "none"
			);
			if (prop.enum_info) {
				std::print(" enum {}", prop.enum_info->name.str());
			}
			std::println("");
		}

		std::println("\tMethods ({}):", info.methods.size());
		for (const auto& method : info.methods) {
			std::println(
					"\t\t{}{} ({})", method.is_static ? "static " : "", method.name.str(), access_name(method.access)
			);
		}
	}

	std::vector<const EnumInfo*> enums;
	for (auto& [name, info] : _enum_infos) {
		enums.push_back(&info);
	}
	std::ranges::sort(enums, {}, [](const EnumInfo* info) { return info->name.str(); });

	for (const EnumInfo* enum_info : enums) {
		std::println("\nEnum: {} ({} bytes)", enum_info->name.str(), enum_info->underlying_size);
		for (const Enumerator& value : enum_info->values) {
			std::println("\t{} = {}", value.name.str(), value.value);
		}
	}
#endif
}

Callable ClassDB::get_static_method(const StaticString& class_name, std::string_view func_name) {
	auto ci = _get_class_info_internal(class_name);
	if (!ci)
		return {};
	auto it = std::ranges::find_if(ci->methods, [&func_name](const ClassInfo::Method& m) {
		return m.is_static && m.name == func_name;
	});
	if (it == ci->methods.end()) {
		return {};
	}
	auto invoke = it->invoke;
	return Callable { [invoke](std::span<Variant> args) { return invoke(nullptr, args); }, it->param_count };
}

} //namespace feather
