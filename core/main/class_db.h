#pragma once

#include <framework/callable.h>
#include <framework/class_info.h>
#include <framework/reflection_utils.h>
#include <framework/singleton_helpers.h>
#include <framework/variant.h>
#include <framework/static_string.hpp>

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace feather {

class ClassDB {
	friend Variant;
	friend struct Main;
	FDECLARE_SINGLETON(ClassDB);

	ClassDB();

	std::map<StaticString, ClassInfo> _class_infos;

	ClassInfo* _current_info = nullptr;

	template <typename T, typename U>
	static constexpr size_t offset_of(U T::* member) {
		return reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member));
	}

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(std::string_view name);

public:
	void print_db();

	template <is_reflected_class_type T>
	static void register_class();

	template <is_reflected_class_type T>
	static void register_abstract_class();

	template <is_reflected_class_type T>
	static void register_singleton_class();

	// Create a property with default accessors
	template <class T, class U>
	static constexpr void bind_property(U T::* member, std::string_view name, VariantType variant_type);

	template <class T, class TRet, class... TArgs>
	static constexpr void bind_method(TRet (T::*method)(TArgs...), std::string_view name);

	template <class T, class TRet, class... TArgs>
	static constexpr void bind_method(TRet (T::*method)(TArgs...) const, std::string_view name);

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

} //namespace feather

#include "class_db.inl"