#pragma once
#include "class_db.h"
#include <framework/variant.h>

#include <concepts>
#include <format>
#include <memory>
#include <string_view>
#include <type_traits>

namespace feather {

template <class T>
concept has_bind_method = requires { T::_bind_members(); };

template <class T>
void ClassDB::register_class() {
	// TODO populate this function
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");
	static_assert(has_bind_method<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = get();

	ClassInfo info { .name = T::get_class_name() };
	info.object_create_func = []() -> Variant { return new T(); };

	instance._class_infos.emplace(std::make_pair(T::get_class_name(), info));
}

} //namespace feather