#pragma once
#include "class_db.h"
#include <string_view>
#include <type_traits>

#include <format>

namespace feather {

template <class T>
void ClassDB::register_class() {
	// TODO populate this function
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");
}

} //namespace feather