module;

#include "export_defs.h"
#include "static_string.hpp"

#include <framework/export_defs.h>
#include <framework/static_string.hpp>

export module feather.core:framework.reflected;

import :framework.reflection_utils;

export namespace feather {
// base class for reflected objects
class FEATHER_API Reflected {
	friend class ClassDB;
	struct _class_type {};

protected:
	using Type = Reflected;

	static void _bind_members();

public:
	virtual ~Reflected() = default;

	constexpr static StaticString get_class_static() { return "Reflected"_ss; }
	constexpr static StaticString get_parent_name() { return ""_ss; }
	inline virtual bool is_of_type(StaticString type_name) const { return get_class_static() == type_name; }
	virtual StaticString get_class_name() = 0;
	template <is_reflected_class_type T>
	bool is_of_type() const {
		return is_of_type(T::get_class_static());
	}
};
} //namespace feather
