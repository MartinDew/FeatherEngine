#pragma once

#include "callable.h"
#include "static_string.hpp"
#include "variant.h"

#include <functional>
#include <vector>

namespace feather {

struct ClassInfo {
	StaticString name = ""_ss;
	StaticString parent = ""_ss;
	std::vector<const ClassInfo*> children;
	struct Property {
		StaticString name;
		// variant type to convert to
		VariantType type;
		size_t member_offset;
		size_t member_size;

		// Function pointers for get/set
		std::function<Variant(void*)> getter; // Takes object pointer, returns Variant
		std::function<void(void*, Variant)> setter; // Takes object pointer and value
	};
	std::vector<Property> properties;

	struct Method {
		StaticString name;
		// Possibly need to store param names later
		Callable callable;
	};

	std::vector<Method> methods;

	std::function<Variant()> object_create_func;
};

} //namespace feather