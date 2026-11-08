module;

#include <framework/singleton_helpers.h>

module feather.core;

import std;

namespace feather {

FSINGLETON_INSTANCE(ClassDB);

ClassDB::ClassDB() {
	FSINGLETON_CONSTRUCT_INSTANCE()
	ClassInfoBuilder<Reflected>::fill(_class_infos[Reflected::get_class_static()], ClassKind::Abstract);
}

const ClassInfo* find_class_info(std::string_view name) {
	return ClassDB::get_class_info(name);
}

ClassDB::~ClassDB() {
	clear();
}

void ClassDB::clear() {
	_instance->_class_infos.clear();
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

ObjectRef ClassDB::instantiate(std::string_view class_name) {
	const ClassInfo* info = get_class_info(class_name);
	if (!info || !info->can_create()) {
		return {};
	}
	void* ptr = allocate_instance(*info);
	if (info->lifecycle.default_construct) {
		try {
			info->lifecycle.default_construct(ptr, 1);
		}
		catch (...) {
			free_instance(ptr, *info);
			throw;
		}
	}
	else {
		// Trivially default-constructible: value-initialization zeroes it.
		std::memset(ptr, 0, info->size);
	}
	return ObjectRef(ptr, info);
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

ClassInfo* ClassDB::_get_class_info_internal(std::string_view name) {
	if (!_instance) {
		return nullptr;
	}
	if (auto it = _instance->_class_infos.find(name); it != _instance->_class_infos.end()) {
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

void ClassDB::_fire_subclass_delegates(std::string_view class_name) {
	ClassInfo* ci = _get_class_info_internal(class_name);
	while (ci && ci->parent != ""_ss) {
		auto it = get()->_subclass_delegates.find(ci->parent);
		if (it != get()->_subclass_delegates.end()) {
			it->second.execute(class_name);
		}
		ci = _get_class_info_internal(ci->parent);
	}
}

bool ClassDB::has_parent(StaticString object_name, StaticString parent_name) {
	ClassInfo* ci = _get_class_info_internal(object_name);
	while (ci && ci->parent != ""_ss) {
		if (ci->parent == parent_name) {
			return true;
		}
		ci = _get_class_info_internal(ci->parent);
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
	auto it = std::find_if(ci->methods.begin(), ci->methods.end(), [&func_name](const ClassInfo::Method& m) {
		return m.name == func_name;
	});

	if (it != ci->methods.end()) {
		return it->callable;
	}
	return {};
}

} //namespace feather
