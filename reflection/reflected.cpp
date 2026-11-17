module feather.reflection;

import std;

namespace feather {

ObjectRef Reflected::_make_object_ref(void* ptr, Reflected* self, StaticString class_name) {
	return ObjectRef(ptr, ClassDB::get_class_info(class_name), self);
}

const ClassInfo* Reflected::get_class_info() {
	return ClassDB::get_class_info(get_class_name());
}

} // namespace feather
