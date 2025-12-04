#pragma once
#include <framework/variant.h>
#include <framework/static_string.hpp>

#include <any>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace feather {

template <class T>
concept is_reflected_class_type = requires { typename T::_class_type; } && std::is_class_v<typename T::_class_type>;

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
		// Todo : function

		std::function<Variant()> object_create_func;
	};

	std::map<StaticString, ClassInfo> _class_infos;

public:
	static ClassDB& get();

	template <class T>
	static void register_class();

	// Returns an unmanaged raw pointer to an object within an any container
	static std::any create_object_unsafe(std::string_view object_name);

	// A somewhat safer function that validates if the requested object can be cast in the specified T
	template <class T>
	static std::unique_ptr<T> create_object(std::string_view object_name) {
		std::any object = create_object_unsafe(object_name);
		std::unique_ptr<T> ptr { std::any_cast<T*>(object) };
		return ptr;
	}
};

} //namespace feather

#include "class_db.inl"