module feather.reflection;

namespace feather {

namespace {
const ObjectAccess* s_object_access = nullptr;
}

void set_object_access(const ObjectAccess* access) {
	s_object_access = access;
}

const ObjectAccess* get_object_access() {
	return s_object_access;
}

} // namespace feather
