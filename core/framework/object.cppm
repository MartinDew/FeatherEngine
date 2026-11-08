module;

#include <framework/export_defs.h>

export module feather.core:framework.object;

import std;
import :framework.static_string;
import :framework.reflection_utils;

export namespace feather {

struct ClassInfo;

// Registered ClassInfo for a class name, or null when unknown or ClassDB isn't up yet.
FEATHER_API const ClassInfo* find_class_info(std::string_view name);

// Non-owning, type-erased reference to a reflected object. ptr always points at the object's most-derived type,
// the one info describes, so it can only be built from a typed reference.
class FEATHER_API ObjectRef {
	friend class Reflected;
	friend class Variant;
	friend class ClassDB;

	void* _ptr = nullptr;
	const ClassInfo* _info = nullptr;

	ObjectRef(void* ptr, const ClassInfo* info) : _ptr { ptr }, _info { info } {}

public:
	ObjectRef() = default;

	// A polymorphic object reports its dynamic type itself. Anything else can't be derived from, so its static type
	// is its dynamic type.
	template <is_reflected T>
		requires(!std::is_const_v<T>)
	ObjectRef(T& object) {
		if constexpr (std::is_polymorphic_v<T>) {
			*this = object._get_object_ref();
		}
		else {
			_ptr = &object;
			_info = find_class_info(T::get_class_static());
		}
	}

	void* get() const noexcept { return _ptr; }
	const ClassInfo* get_class_info() const noexcept { return _info; }
	explicit operator bool() const noexcept { return _ptr != nullptr; }
	bool operator==(const ObjectRef&) const = default;

	StaticString get_class_name() const;
	bool is_a(const ClassInfo* target) const;
	// Pointer to the target base subobject, or null when the object isn't one.
	void* cast_to(const ClassInfo* target) const;

	template <is_reflected T>
	T* cast() const {
		return static_cast<T*>(cast_to(find_class_info(std::remove_const_t<T>::get_class_static())));
	}
};

} // namespace feather
