module;

#include "export_defs.h"

#include <framework/export_defs.h>

export module feather.core:framework.reflected;

export import :framework.reflection_utils;
export import :framework.object;
import :framework.static_string;

export namespace feather {
// base class for reflected objects
class FEATHER_API Reflected {
	friend class ClassDB;
	struct _class_type {};

protected:
	using Type = Reflected;

	// For the generated _get_object_ref overrides: `this` there is the most-derived type.
	static ObjectRef _make_object_ref(void* ptr, StaticString class_name) {
		return ObjectRef(ptr, find_class_info(class_name));
	}

public:
	virtual ~Reflected() = default;

	constexpr static StaticString get_class_static() { return "Reflected"_ss; }
	constexpr static StaticString get_parent_name() { return ""_ss; }
	inline virtual bool is_of_type(StaticString type_name) const { return get_class_static() == type_name; }
	virtual StaticString get_class_name() = 0;
	virtual ObjectRef _get_object_ref() = 0;
	const ClassInfo* get_class_info() { return find_class_info(get_class_name()); }
	template <is_reflected_class_type T>
	bool is_of_type() const {
		return is_of_type(T::get_class_static());
	}
};
} //namespace feather
