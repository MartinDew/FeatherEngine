#pragma once

#include <framework/reflection_utils.h>
#include <framework/variant.h>
#include <framework/static_string.hpp>

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace feather {

class ClassDB {
	static std::unique_ptr<ClassDB> _instance;
	ClassDB() = default;

	struct ClassInfo {
		StaticString name;
		struct Property {
			StaticString name;
			size_t type;
			size_t member_offset;
			size_t member_size;
		};
		std::vector<Property> properties;
		// Todo : functions

		std::function<Variant()> object_create_func;
	};

	std::map<StaticString, ClassInfo> _class_infos;

public:
	static ClassDB& get();

	template <is_reflected_class_type T>
	static void register_class();

	// Returns an unmanaged raw pointer to a reflected object
	static Reflected* create_object_unsafe(std::string_view object_name);

	// A somewhat safer function that validates if the requested object can be cast in the specified T
	template <class T>
	static std::unique_ptr<T> create_object(std::string_view object_name) {
		Reflected* object = create_object_unsafe(object_name);
		std::unique_ptr<T> ptr { object_cast<T>(object) };
		return ptr;
	}
};

} //namespace feather

#include "class_db.inl"