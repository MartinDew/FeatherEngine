#pragma once
#include "class_db.h"

#include "main/class_db.inl"
#include <framework/reflection_utils.h>
#include <framework/variant.h>

#include <concepts>
#include <type_traits>

namespace feather {

template <class T>
void has_bind_method(const T& t) {
	t._bind_members();
}

template <typename T>
concept has_bind_method_v = requires(T t) {
	{
		has_bind_method(t)
	};
};

template <is_reflected_class_type T>
void ClassDB::register_class() {
	// TODO populate this function
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = get();

	ClassInfo new_info { .name = T::get_class_name() };
	new_info.object_create_func = []() -> Variant { return new T(); };

	instance._class_infos.emplace(std::make_pair(T::get_class_name(), new_info));
	auto& info = instance._class_infos.at(T::get_class_name());
	instance._current_info = &info;

	T::_bind_members();

	instance._current_info = nullptr;
}

} //namespace feather