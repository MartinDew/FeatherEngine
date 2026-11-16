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
}

// Allocates the way a new-expression would, so a Reflected instance can also be freed with a plain delete.
static void* allocate_instance(const ClassInfo& info) {
	if (info.alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		return ::operator new(info.size, std::align_val_t { info.alignment });
	}
	return ::operator new(info.size);
}

static void free_instance(void* ptr, const ClassInfo& info) {
	if (info.alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		::operator delete(ptr, info.size, std::align_val_t { info.alignment });
	}
	else {
		::operator delete(ptr, info.size);
	}
}

ObjectRef ClassDB::instantiate(StaticString class_name) {
	const ClassInfo* info = get_class_info(class_name);
	if (!info || !info->can_create()) {
		return {};
	}
	void* ptr = allocate_instance(*info);
	if (info->lifecycle.default_construct) {
		try {
			info->lifecycle.default_construct(ptr, 1);
		} catch (...) {
			free_instance(ptr, *info);
			throw;
		}
	}
	else {
		// Trivially default-constructible: value-initialization zeroes it.
		std::memset(ptr, 0, info->size);
	}
	ObjectRef object(ptr, info);
	object._reflected = static_cast<Reflected*>(object.cast_to(get_class_info(Reflected::get_class_static())));
	return object;
}

void ClassDB::destroy(ObjectRef object) {
	if (!object) {
		return;
	}
	const ClassInfo& info = *object.get_class_info();
	if (info.lifecycle.destruct) {
		info.lifecycle.destruct(object.get(), 1);
	}
	free_instance(object.get(), info);
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

void ClassDB::print_db() {
#ifdef BETA
	std::println("Printing database");

	std::vector<const ClassInfo*> sorted;
	for (auto& [name, info] : _class_infos) {
		sorted.push_back(&info);
	}
	std::ranges::sort(sorted, {}, [](const ClassInfo* info) { return info->name.str(); });

	for (const ClassInfo* class_info : sorted) {
		const ClassInfo& info = *class_info;
		StaticString name = info.name;
		std::println("Class: {} : ", info.name.str());
		for (auto& prop : info.properties) {
			std::println("\tProperty: {} Type: {}\n", prop.name.str(), std::to_underlying<VariantType>(prop.type));
		}

		std::println("Children : {}", get_children_names_string(name, false));
	}
#endif
}

Callable ClassDB::get_static_method(const StaticString& class_name, std::string_view func_name) {
	auto ci = _get_class_info_internal(class_name);
	if (!ci)
		return {};
	auto it = std::find_if(ci->methods.begin(), ci->methods.end(), [&func_name](ClassInfo::Method& m) {
		// An instance method's callable expects the object first, which a static call can't supply.
		return m.is_static && m.name == func_name;
	});

	if (it != ci->methods.end()) {
		return it->callable;
	}
	return {};
}

} //namespace feather
