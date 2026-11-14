module feather.reflection;

import std;

namespace feather {

const ClassInfo* find_class_info(std::string_view name) {
	return ClassDB::get() ? ClassDB::get_class_info(name) : nullptr;
}

StaticString ObjectRef::get_class_name() const {
	return _info ? _info->name : ""_ss;
}

bool ObjectRef::is_a(const ClassInfo* target) const {
	return cast_to(target) != nullptr;
}

void* ObjectRef::cast_to(const ClassInfo* target) const {
	if (!target || !_info) {
		return nullptr;
	}
	void* result = nullptr;
	_info->walk(_ptr, [&](const ClassInfo& info, void* object) {
		if (&info == target) {
			result = object;
			return true;
		}
		return false;
	});
	return result;
}

} // namespace feather
