export module feather.core:framework.reflection_utils;

import std;
import :framework.static_string;

export namespace feather {

// Forward-declared, not included: reflected.h includes this header before
// Reflected is defined, so including it back here would be circular.
// is_base_of_v below is a dependent expression, only resolved once a concrete
// T is checked -- by then reflected.h is fully included wherever it matters.
class Reflected;

template <class T>
concept is_reflected_class_type = requires { T::get_class_static(); };

// Value type (FSTRUCT / FCLASS(novtable)): reflected but not derived from
// Reflected, so no vtable pointer -- see ClassDB::register_class.
template <class T>
concept is_reflected_value_type = is_reflected_class_type<T> && !std::is_base_of_v<Reflected, T>;

// Mirrors the C++ access of a member (or an explicit property attribute); gates
// script/editor access via Variant::get/set/call.
enum class AccessLevel : std::uint8_t {
	Public = 0,
	Protected,
	Private,
};

inline constexpr std::size_t no_offset = static_cast<std::size_t>(-1);

// Compile-time description of one property, emitted by codegen into a type's _feather_properties(). Getter and
// Setter are each nullptr, a data member pointer, or an accessor member function pointer.
template <auto Getter, auto Setter>
struct PropertyDesc {
	static constexpr auto getter = Getter;
	static constexpr auto setter = Setter;

	StaticString name;
	AccessLevel getter_access = AccessLevel::Public;
	AccessLevel setter_access = AccessLevel::Public;
	// Byte offset of the backing field; only known for a value type's plain field.
	std::size_t offset = no_offset;
};

// Compile-time description of one method: a member function pointer or a static function pointer.
template <auto Function>
struct MethodDesc {
	static constexpr auto function = Function;

	StaticString name;
	AccessLevel access = AccessLevel::Public;
};

template <class T, is_reflected_class_type T2>
	requires is_reflected_class_type<std::remove_const_t<T>>
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

template <class T, is_reflected_class_type T2>
T* object_cast(T2& object) {
	return object_cast<T>(&object);
}

} // namespace feather
