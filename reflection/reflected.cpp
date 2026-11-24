module feather.reflection;

import std;

namespace feather {

const ClassInfo* Reflected::get_class_info() {
	return ClassDB::get_class_info(get_class_name());
}

} // namespace feather
