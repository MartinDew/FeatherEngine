module feather.core;

import std;

namespace feather {

StaticString ObjectRef::get_class_name() const {
	return _info ? _info->name : ""_ss;
}

bool ObjectRef::is_a(const ClassInfo* target) const {
	return cast_to(target) != nullptr;
}

void* ObjectRef::cast_to(const ClassInfo* target) const {
	if (!target) {
		return nullptr;
	}
	const ClassInfo* info = _info;
	void* object = _ptr;
	while (info && object) {
		if (info == target) {
			return object;
		}
		if (!info->to_parent) {
			return nullptr;
		}
		object = info->to_parent(object);
		info = ClassDB::get_class_info(info->parent);
	}
	return nullptr;
}

} // namespace feather
