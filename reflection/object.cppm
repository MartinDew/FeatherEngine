export module feather.reflection:object;

import std;
import feather.framework;
import :reflection_utils;

export namespace feather {

struct ClassInfo;

// Non-owning, type-erased reference to a reflected object, FCLASS or FSTRUCT alike. ptr always points at the
// object's most-derived type, the one info describes, so it can only be built from a typed reference.
class FEATHER_REFLECTION_API ObjectRef {
	friend class Reflected;
	friend class Variant;
	friend class ClassDB;

	void* _ptr = nullptr;
	const ClassInfo* _info = nullptr;
	// The same object as its Reflected base, or null for a value type. Lets Reflected casts go through the virtual
	// is_of_type instead of the registered hierarchy.
	Reflected* _reflected = nullptr;

	ObjectRef(void* ptr, const ClassInfo* info, Reflected* reflected = nullptr)
			: _ptr { ptr }
			, _info { info }
			, _reflected { reflected } {}

public:
	ObjectRef() = default;

	// A Reflected object reports its dynamic type itself. Nothing derives from a value type, so its static type is
	// its dynamic type. Defined with ClassDB (class_db.cppm).
	template <class T>
		requires(!std::is_const_v<T> && (std::is_base_of_v<Reflected, T> || is_reflected<T>))
	ObjectRef(T& object);

	void* get() const noexcept { return _ptr; }
	const ClassInfo* get_class_info() const noexcept { return _info; }
	Reflected* get_reflected() const noexcept { return _reflected; }
	explicit operator bool() const noexcept { return _ptr != nullptr; }
	bool operator==(const ObjectRef& other) const noexcept { return _ptr == other._ptr && _info == other._info; }

	StaticString get_class_name() const;
	bool is_a(const ClassInfo* target) const;
	// Pointer to the target base subobject, or null when the object isn't one.
	void* cast_to(const ClassInfo* target) const;

	// Null when the object isn't a T. Defined with ClassDB (class_db.cppm).
	template <is_reflected T>
	T* cast() const;
};

} // namespace feather
