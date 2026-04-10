#pragma once

#include <framework/reflected.h>
#include <type_traits>

namespace feather {

template <class T>
concept is_reflected_class_type = requires { T::get_class_static(); };

template <class T, is_reflected_class_type T2>
T* object_cast(T2* object) {
	if constexpr (std::is_const_v<T2>)
		static_assert(std::is_const_v<T>, "Return type must be const for const object casts");

	if (object == nullptr) {
		return nullptr;
	}
	if (object->is_of_type(T2::get_class_static())) {
		return static_cast<T*>(object);
	}
	return nullptr;
}

template <class T, is_reflected_class_type T2>
T* object_cast(T2& object) {
	return object_cast<T>(&object);
}

} // namespace feather