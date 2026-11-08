export module feather.core:framework.reflection_utils;

import std;
import :framework.static_string;

export namespace feather {

class Reflected;

// T carries its own reflection (FCLASS/FSTRUCT), not just a reflected base's: _feather_self is redeclared by every
// reflected class, so a plain subclass of one doesn't match.
template <class T>
concept is_reflected = requires { typename T::_feather_self; } && std::is_same_v<typename T::_feather_self, std::remove_cv_t<T>>;

// Mirrors the C++ access of a member (or an explicit property attribute); gates
// script/editor access via Variant::get/set/call.
enum class AccessLevel : std::uint8_t {
	Public = 0,
	Protected,
	Private,
};

// Compile-time description of one property, emitted by codegen into a type's _feather_properties(). Getter and
// Setter are each nullptr, a data member pointer, or an accessor member function pointer.
template <auto Getter, auto Setter>
struct PropertyDesc {
	static constexpr auto getter = Getter;
	static constexpr auto setter = Setter;

	StaticString name;
	AccessLevel getter_access = AccessLevel::Public;
	AccessLevel setter_access = AccessLevel::Public;
};

// Compile-time description of one method: a member function pointer or a static function pointer.
template <auto Function>
struct MethodDesc {
	static constexpr auto function = Function;

	StaticString name;
	AccessLevel access = AccessLevel::Public;
};

template <class T, is_reflected T2>
	requires is_reflected<T>
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
