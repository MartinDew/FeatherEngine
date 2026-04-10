#pragma once
#include "class_db.h"

#include "framework/callable.h"
#include <framework/functions.h>
#include <framework/reflection_utils.h>
#include <framework/singleton_helpers.h>
#include <framework/variant.h>

#include <concepts>
#include <type_traits>

namespace feather {

template <class T>
void has_bind_method(const T& t) {
	t._bind_members();
}

template <typename T>
concept has_bind_method_v = requires(T t) {
	{ has_bind_method(t) };
};

template <is_reflected_class_type T>
void ClassDB::register_class() {
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	if constexpr (std::is_abstract_v<T>) {
		ClassDB::register_abstract_class<T>();
	}
	else if constexpr (is_singleton_v<T>) {
		ClassDB::register_singleton_class<T>();
	}
	else {
		static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");
		static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
		ClassDB& instance = get();

		ClassInfo& info = instance._class_infos[T::get_class_static()];
		info.name = T::get_class_static();
		info.parent = T::get_parent_name();
		info.object_create_func = []() -> Variant { return new T(); };

		instance._current_info = &info;

		if (T::get_parent_name() != "") {
			instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
		}

		T::_bind_members();

		instance._current_info = nullptr;
	}
}

template <is_reflected_class_type T> void ClassDB::register_abstract_class() {
	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = get();

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.object_create_func = nullptr;

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
}

template <is_reflected_class_type T> void ClassDB::register_singleton_class() {
	// Todo implement singleton specific logic
	register_abstract_class<T>();
}

// Property binding

template <class T, class U>
inline constexpr void ClassDB::bind_property(U T::* member, std::string_view name, VariantType variant_type) {
	if (!get()._current_info) {
		return;
	}

	ClassInfo::Property prop {
		.name = StaticString(name),
		.type = variant_type,
	};

	// Getter : takes void*(will be cast to T*), returns Variant
	prop.getter = [member](void* obj_ptr) -> Variant {
		T* typed_ptr = static_cast<T*>(obj_ptr);
		return Variant(typed_ptr->*member);
	};

	// Setter: takes void* and Variant, sets the member
	prop.setter = [member](void* obj_ptr, Variant val) {
		T* typed_ptr = static_cast<T*>(obj_ptr);
		typed_ptr->*member = val.as<U>().value();
	};

	get()._current_info->properties.push_back(std::move(prop));
}

// Method binding
template <class T, class TRet, class... TArgs>
inline constexpr void ClassDB::bind_method(TRet (T::*method)(TArgs...), std::string_view name) {
	if (!get()._current_info) {
		return;
	}

	// Create a function that takes T* as first parameter, then the method args
	std::function<TRet(T*, TArgs...)> func = [method](Reflected* instance, TArgs... args) -> TRet {
		return (object_cast<T>(instance)->*method)(args...);
	};

	ClassInfo::Method method_info { .name = StaticString(name), .callable = Callable { func } };

	get()._current_info->methods.push_back(std::move(method_info));
}

template <class T, class TRet, class... TArgs>
inline constexpr void ClassDB::bind_method(TRet (T::*method)(TArgs...) const, std::string_view name) {
	if (!get()._current_info) {
		return;
	}

	// Create a function that takes T* as first parameter, then the method args
	std::function<TRet(T*, TArgs...)> func = [method](Reflected* instance, TArgs... args) -> TRet {
		return (object_cast<T>(instance)->*method)(args...);
	};

	ClassInfo::Method method_info { .name = StaticString(name), .callable = Callable { func } };

	get()._current_info->methods.push_back(std::move(method_info));
}

} //namespace feather