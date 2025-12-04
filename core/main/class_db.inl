#pragma once
#include "class_db.h"
#include <concepts>
#include <string_view>
#include <type_traits>

#include <format>

namespace feather {

template <class T>
concept has_bind_method = requires { T::_bind_members(); };

template <class T>
void ClassDB::register_class() {
	// TODO populate this function
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");

	static_assert(has_bind_method<T>, "Class doesn't have a static _bind_members function");
}

} //namespace feather