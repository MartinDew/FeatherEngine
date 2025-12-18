#pragma once

#include <type_traits>

template <class T>
concept is_reflected_class_type = requires { T::get_class_static(); };

template <class T, is_reflected_class_type T2>
T* object_cast(T2* object) {
	if (object == nullptr) {
		return nullptr;
	}
	if (object->template is_of_type<T>()) {
		return static_cast<T*>(object);
	}
	return nullptr;
}

template <class T, is_reflected_class_type T2>
T* object_cast(T2& object) {
	if (object.template is_of_type<T>()) {
		return static_cast<T*>(&object);
	}
	return nullptr;
}