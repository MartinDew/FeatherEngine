module feather.reflection;

import std;

namespace feather {

ObjectRef ObjectRef::from_value_storage(void* ptr, const ClassInfo& info) {
	return ptr && info.is_value_type ? ObjectRef(ptr, &info) : ObjectRef {};
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
