#pragma once

#include <framework/reflection_utils.h>
#include <framework/variant.h>
#include <framework/static_string.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace feather {

class ClassDB {
	static std::unique_ptr<ClassDB> _instance;
	ClassDB();

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
		// Todo : functions

		std::function<Variant()> object_create_func;
	};

	std::map<StaticString, ClassInfo> _class_infos;

	ClassInfo* _current_info = nullptr;

	template <typename T, typename U>
	static constexpr size_t offset_of(U T::* member) {
		return reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member));
	}

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

public:
	static ClassDB& get();

	void print_db();

	template <is_reflected_class_type T>
	static void register_class();

	template <is_reflected_class_type T>
	static void register_abstract_class();

	template <class T, class U>
	static constexpr void bind_property(U T::* member, std::string_view name, VariantType variant_type);

	// Returns an unmanaged raw pointer to a reflected object
	static Reflected* create_object_unsafe(std::string_view object_name);

	// A somewhat safer function that validates if the requested object can be cast in the specified T
	template <class T>
	static std::unique_ptr<T> create_object(std::string_view object_name) {
		Reflected* object = create_object_unsafe(object_name);
		std::unique_ptr<T> ptr { object_cast<T>(object) };
		return ptr;
	}

	static std::vector<StaticString> get_children_names(std::string_view object_name, bool exclusive = false);

	static std::string get_children_names_string(StaticString object_name, bool exclusive = false);
};

template <class T, class U>
constexpr void ClassDB::bind_property(U T::* member, std::string_view name, VariantType variant_type) {
	if (!get()._current_info) {
		return;
	}

	ClassInfo::Property prop {
		.name = StaticString(name),
		.type = variant_type,
		.member_offset = offset_of(member),
		.member_size = sizeof(U),
	};

	// Getter : takes void*(will be cast to T*), returns Variant
	prop.getter = [member](void* obj_ptr) -> Variant {
		T* typed_ptr = static_cast<T*>(obj_ptr);
		return Variant(typed_ptr->*member);
	};

	// Setter: takes void* and Variant, sets the member
	prop.setter = [member](void* obj_ptr, Variant val) {
		T* typed_ptr = static_cast<T*>(obj_ptr);
		typed_ptr->*member = val.get<U>().value();
	};

	get()._current_info->properties.push_back(std::move(prop));
}

} //namespace feather

#include "class_db.inl"