#include "extension.h"

namespace feather {

Extension::Extension(const std::string_view& name, const std::function<void()> register_func)
		: _extension_name(name)
		, _register_func(register_func) {
}

void Extension::_bind_members() {
}

const bool Extension::register_extension() const {
	_register_func();
	return true;
}

const bool Extension::unregister_extension() const {
	fassert(false, "Unregistering extensions is not supported yet");
	return false;
}

} // namespace feather
