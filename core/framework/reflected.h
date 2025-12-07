#pragma once

#include "reflection_utils.h"
#include "static_string.hpp"

namespace feather {
// base class for reflected objects
class Reflected {
	friend class ClassDB;
	struct _class_type {};

protected:
	using Type = Reflected;

	static void _bind_members();

public:
	constexpr static StaticString get_class_name() { return "Reflected"_ss; }
	constexpr static StaticString get_parent_name() { return ""_ss; }
	inline virtual bool is_of_type(StaticString type_name) { return get_class_name() == type_name; }

	template <is_reflected_class_type T>
	bool is_of_type() {
		return is_of_type(T::get_class_name());
	}
};
} //namespace feather