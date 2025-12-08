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

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.object_create_func = []() -> Variant { return new T(); };

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
}

template <is_reflected_class_type T> void ClassDB::register_abstract_class() {
	// TODO populate this function
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = get();

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.object_create_func = nullptr;

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
}

} //namespace feather