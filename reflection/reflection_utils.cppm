export module feather.reflection:reflection_utils;

import std;

export namespace feather {

class Reflected;

// T carries its own reflection (FCLASS/FSTRUCT), not just a reflected base's: Type is redeclared by every reflected
// class, so a plain subclass of one doesn't match.
template <class T>
concept is_reflected = requires { typename T::Type; } && std::is_same_v<typename T::Type, std::remove_cv_t<T>>;

// Value type (FSTRUCT / FCLASS(novtable)): reflected but not derived from Reflected, so no vtable pointer.
template <class T>
concept is_reflected_value_type = is_reflected<T> && !std::is_base_of_v<Reflected, T>;

template <class T, is_reflected T2>
	requires is_reflected<std::remove_const_t<T>>
T* object_cast(T2* object) {
	if constexpr (std::is_const_v<T2>)
		static_assert(std::is_const_v<T>, "Return type must be const for const object casts");

	if (object == nullptr) {
		return nullptr;
	}
	if (object->is_of_type(std::remove_const_t<T>::get_class_static())) {
		return static_cast<T*>(object);
	}
	return nullptr;
}

template <class T, is_reflected T2>
T* object_cast(T2& object) {
	return object_cast<T>(&object);
}

} // namespace feather
