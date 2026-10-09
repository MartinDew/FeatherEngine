module;

#include <framework/export_defs.h>
#include <framework/singleton_helpers.h>
#include <framework/static_string.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <print>
#include <type_traits>
#include <vector>

export module feather.core:main.class_db;

import :framework.callable;
import :framework.class_info;
import :framework.reflection_utils;
import :framework.variant;
import :framework.delegate;
import :framework.functions;

export namespace feather {

class FEATHER_API ClassDB {
	friend Variant;
	friend struct Main;
	FDECLARE_SINGLETON(ClassDB);

	ClassDB();
	~ClassDB();

public:
	using subclass_delegate_t = Delegate<const std::string_view>;

private:
	std::map<StaticString, ClassInfo> _class_infos;
	std::map<StaticString, subclass_delegate_t> _subclass_delegates;

	ClassInfo* _current_info = nullptr;

	static void _fire_subclass_delegates(std::string_view class_name);

	template <typename T, typename U>
	static constexpr size_t offset_of(U T::* member) {
		return reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member));
	}

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(std::string_view name);

public:
	void print_db();

	// Empties the class registry. Must run before any extension DLL that
	// registered classes gets unloaded -- see Main::~Main().
	static void clear();

	static Callable get_static_method(const StaticString& class_name, std::string_view func_name);

	template <is_reflected_class_type T>
	static void register_class();

	template <is_reflected_class_type T>
	static void register_abstract_class();

	template <is_reflected_class_type T>
	static void register_singleton_class();

	// Value type (FSTRUCT / FCLASS(novtable)): properties/static methods only,
	// no Reflected base or object_create_func -- no vtable to factory through.
	template <is_reflected_value_type T>
		requires(!std::is_base_of_v<Reflected, T>)
	static void register_value_class();

	// Property backed directly by a data member; both accessors share its access level.
	template <class T, class U>
	static void bind_property(U T::* member, std::string_view name, AccessLevel access = AccessLevel::Public);

	// Property backed by explicit getter/setter member functions, with
	// independent accessibility per accessor. Read-only/write-only overloads
	// exist because a null member pointer can't be deduced.
	template <class T, class TGet, class TSet>
	static void bind_property_accessors(
			TGet (T::*getter)() const,
			void (T::*setter)(TSet),
			std::string_view name,
			AccessLevel getter_access = AccessLevel::Public,
			AccessLevel setter_access = AccessLevel::Public
	);

	template <class T, class TGet>
	static void
	bind_property_get(TGet (T::*getter)() const, std::string_view name, AccessLevel access = AccessLevel::Public);

	template <class T, class TSet>
	static void
	bind_property_set(void (T::*setter)(TSet), std::string_view name, AccessLevel access = AccessLevel::Public);

	// Guarded binds: no-op when the property type isn't Variant-marshalable
	// (e.g. std::shared_ptr<...>), so generated accessors never break the build.
	template <class T, class TGet, class TSet>
	static void bind_property_accessors_if_bindable(
			TGet (T::*getter)() const,
			void (T::*setter)(TSet),
			std::string_view name,
			AccessLevel getter_access = AccessLevel::Public,
			AccessLevel setter_access = AccessLevel::Public
	);

	template <class T, class TGet>
	static void bind_property_get_if_bindable(
			TGet (T::*getter)() const,
			std::string_view name,
			AccessLevel access = AccessLevel::Public
	);

	template <class T, class TSet>
	static void bind_property_set_if_bindable(
			void (T::*setter)(TSet),
			std::string_view name,
			AccessLevel access = AccessLevel::Public
	);

	template <class T, class TRet, class... TArgs>
	static void
	bind_method(TRet (T::*method)(TArgs...), std::string_view name, AccessLevel access = AccessLevel::Public);

	template <class T, class TRet, class... TArgs>
	static void
	bind_method(TRet (T::*method)(TArgs...) const, std::string_view name, AccessLevel access = AccessLevel::Public);

	template <class TRet, class... TArgs>
	static void
	bind_static_method(TRet (*method)(TArgs...), std::string_view name, AccessLevel access = AccessLevel::Public);

	// Guarded bind_method: no-op when the signature isn't Variant-marshalable.
	// Used for auto-bound (opt-out) methods; explicit [[method]] uses bind_method directly.
	template <class T, class TRet, class... TArgs>
	static void bind_method_if_bindable(
			TRet (T::*method)(TArgs...),
			std::string_view name,
			AccessLevel access = AccessLevel::Public
	);

	template <class T, class TRet, class... TArgs>
	static void bind_method_if_bindable(
			TRet (T::*method)(TArgs...) const,
			std::string_view name,
			AccessLevel access = AccessLevel::Public
	);

	// Returns an unmanaged raw pointer to a reflected object
	static Reflected* create_object_unsafe(std::string_view object_name);

	// A somewhat safer function that validates if the requested object can be cast in the specified T
	template <class T>
	static std::unique_ptr<T> create_object(std::string_view object_name) {
		Reflected* object = create_object_unsafe(object_name);
		std::unique_ptr<T> ptr { object_cast<T>(object) };
		return ptr;
	}

	static Delegate<std::string_view>::id_t on_subclass_registered(
			std::string_view base_class_name,
			const Delegate<std::string_view>::DelegateFuncType& callback
	);

	static void unregister_subclass_delegate(std::string_view base_class_name, Delegate<std::string_view>::id_t id);

	static std::vector<StaticString> get_children_names(std::string_view object_name, bool exclusive = false);

	static std::string get_children_names_string(StaticString object_name, bool exclusive = false);

	static bool has_parent(StaticString object_name, StaticString parent_name);
};

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
	if constexpr (std::is_abstract_v<T> || !std::is_default_constructible_v<T>) {
		ClassDB::register_abstract_class<T>();
	}
	else if constexpr (is_singleton_v<T>) {
		ClassDB::register_singleton_class<T>();
	}
	else {
		static_assert(std::is_default_constructible<T>(), "Trying to register class that is not default constructible");
		static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
		std::println("Registering class '{}' as {} object", T::get_class_static(), "implementation");
		ClassDB& instance = *get();

		ClassInfo& info = instance._class_infos[T::get_class_static()];
		info.name = T::get_class_static();
		info.parent = T::get_parent_name();
		info.is_abstract = false;
		info.is_singleton = false;
		info.object_create_func = []() -> Variant { return new T(); };

		instance._current_info = &info;

		if (T::get_parent_name() != "") {
			instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
		}

		T::_bind_members();

		instance._current_info = nullptr;
		_fire_subclass_delegates(T::get_class_static());
	}
}

template <is_reflected_class_type T> void ClassDB::register_abstract_class() {
	std::println("Registering class '{}' as {} object", T::get_class_static(), "abstract");

	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = *get();

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.is_abstract = true;
	info.is_singleton = false;
	info.object_create_func = nullptr;

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
	_fire_subclass_delegates(T::get_class_static());
}

template <is_reflected_class_type T> void ClassDB::register_singleton_class() {
	std::println("Registering class '{}' as {} object", T::get_class_static(), "singleton");

	static_assert(is_reflected_class_type<T>, "Attempt to register a non reflected class type");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = *get();

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.is_abstract = false;
	info.is_singleton = true;
	// Singletons are not created through the reflection factory; the engine owns
	// the single instance and constructs it in its own flow. Reflection resolves
	// live instances through T::get().
	info.object_create_func = nullptr;

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
	_fire_subclass_delegates(T::get_class_static());
}

template <is_reflected_value_type T>
	requires(!std::is_base_of_v<Reflected, T>)
void ClassDB::register_value_class() {
	std::println("Registering class '{}' as {} object", T::get_class_static(), "value type");

	static_assert(is_reflected_value_type<T>, "Attempt to register a non-value reflected class type");
	static_assert(has_bind_method_v<T>, "Class doesn't have a static _bind_members function");
	ClassDB& instance = *get();

	ClassInfo& info = instance._class_infos[T::get_class_static()];
	info.name = T::get_class_static();
	info.parent = T::get_parent_name();
	info.is_abstract = false;
	info.is_singleton = false;
	info.is_value_type = true;
	// No Reflected base means no polymorphic factory -- Variant's pointer path
	// requires std::is_base_of_v<Reflected, T>, which a value type never satisfies.
	info.object_create_func = nullptr;

	instance._current_info = &info;

	if (T::get_parent_name() != "") {
		instance._class_infos[T::get_parent_name()].children.emplace_back(&info);
	}

	T::_bind_members();

	instance._current_info = nullptr;
	_fire_subclass_delegates(T::get_class_static());
}

// Property binding

template <class T, class U>
inline void ClassDB::bind_property(U T::* member, std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}

	ClassInfo::Property prop {
		.name = StaticString(name), .type = get_variant_type<U>(), .getter_access = access, .setter_access = access
	};

	// Getter : takes void*(will be cast to T*), returns Variant
	prop.getter = [member](void* obj_ptr) -> Variant {
		T* typed_ptr = static_cast<T*>(obj_ptr);
		return Variant(typed_ptr->*member);
	};

	// Setter: takes void* and Variant, sets the member
	prop.setter = [member, name](void* obj_ptr, Variant val) {
		auto result = val.as<U>();
		fassert(result.has_value(), std::format("Property '{}': setter called with incompatible Variant type", name));
		static_cast<T*>(obj_ptr)->*member = std::move(result.value());
	};

	get()->_current_info->properties.push_back(std::move(prop));
}

// Property backed by explicit getter + setter member functions.
template <class T, class TGet, class TSet>
inline void ClassDB::bind_property_accessors(TGet (T::*getter)() const,
											 void (T::*setter)(TSet),
											 std::string_view name,
											 AccessLevel getter_access,
											 AccessLevel setter_access) {
	if (!get()->_current_info) {
		return;
	}
	using U = std::decay_t<TGet>;

	ClassInfo::Property prop { .name = StaticString(name),
							   .type = get_variant_type<U>(),
							   .getter_access = getter_access,
							   .setter_access = setter_access };

	prop.getter = [getter](void* obj_ptr) -> Variant { return Variant((static_cast<T*>(obj_ptr)->*getter)()); };
	prop.setter = [setter, name](void* obj_ptr, Variant val) {
		auto result = val.as<std::decay_t<TSet>>();
		fassert(result.has_value(), std::format("Property '{}': setter called with incompatible Variant type", name));
		(static_cast<T*>(obj_ptr)->*setter)(std::move(result.value()));
	};

	get()->_current_info->properties.push_back(std::move(prop));
}

// Read-only property (getter only).
template <class T, class TGet>
inline void ClassDB::bind_property_get(TGet (T::*getter)() const, std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}
	using U = std::decay_t<TGet>;

	ClassInfo::Property prop { .name = StaticString(name), .type = get_variant_type<U>(), .getter_access = access };
	prop.getter = [getter](void* obj_ptr) -> Variant { return Variant((static_cast<T*>(obj_ptr)->*getter)()); };

	get()->_current_info->properties.push_back(std::move(prop));
}

// Write-only property (setter only).
template <class T, class TSet>
inline void ClassDB::bind_property_set(void (T::*setter)(TSet), std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}
	using U = std::decay_t<TSet>;

	ClassInfo::Property prop { .name = StaticString(name), .type = get_variant_type<U>(), .setter_access = access };
	prop.setter = [setter, name](void* obj_ptr, Variant val) {
		auto result = val.as<std::decay_t<TSet>>();
		fassert(result.has_value(), std::format("Property '{}': setter called with incompatible Variant type", name));
		(static_cast<T*>(obj_ptr)->*setter)(std::move(result.value()));
	};

	get()->_current_info->properties.push_back(std::move(prop));
}

// Method binding
template <class T, class TRet, class... TArgs>
inline void ClassDB::bind_method(TRet (T::*method)(TArgs...), std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}

	// Create a function that takes T* as first parameter, then the method args
	std::function<TRet(Reflected*, TArgs...)> func = [method](Reflected* instance, TArgs... args) -> TRet {
		return (object_cast<T>(instance)->*method)(args...);
	};

	ClassInfo::Method method_info { .name = StaticString(name), .access = access, .callable = Callable { func } };

	get()->_current_info->methods.push_back(std::move(method_info));
}

template <class T, class TRet, class... TArgs>
inline void ClassDB::bind_method(TRet (T::*method)(TArgs...) const, std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}

	// Create a function that takes T* as first parameter, then the method args
	std::function<TRet(Reflected*, TArgs...)> func = [method](Reflected* instance, TArgs... args) -> TRet {
		return (object_cast<T>(instance)->*method)(args...);
	};

	ClassInfo::Method method_info { .name = StaticString(name), .access = access, .callable = Callable { func } };

	get()->_current_info->methods.push_back(std::move(method_info));
}

template <class TRet, class... TArgs>
inline void ClassDB::bind_static_method(TRet (*method)(TArgs...), std::string_view name, AccessLevel access) {
	if (!get()->_current_info) {
		return;
	}

	// No instance parameter needed for static functions
	std::function<TRet(TArgs...)> func = [method](TArgs... args) -> TRet { return method(args...); };

	ClassInfo::Method method_info { .name = StaticString(name), .access = access, .callable = Callable { func } };

	get()->_current_info->methods.push_back(std::move(method_info));
}

// Guarded property binds — no-op when the property type isn't Variant-marshalable.
template <class T, class TGet, class TSet>
inline void ClassDB::bind_property_accessors_if_bindable(TGet (T::*getter)() const,
														 void (T::*setter)(TSet),
														 std::string_view name,
														 AccessLevel getter_access,
														 AccessLevel setter_access) {
	if constexpr (VariantCompatible<std::decay_t<TGet>> && VariantCompatible<std::decay_t<TSet>>) {
		bind_property_accessors(getter, setter, name, getter_access, setter_access);
	}
}

template <class T, class TGet>
inline void
ClassDB::bind_property_get_if_bindable(TGet (T::*getter)() const, std::string_view name, AccessLevel access) {
	if constexpr (VariantCompatible<std::decay_t<TGet>>) {
		bind_property_get(getter, name, access);
	}
}

template <class T, class TSet>
inline void ClassDB::bind_property_set_if_bindable(void (T::*setter)(TSet), std::string_view name, AccessLevel access) {
	if constexpr (VariantCompatible<std::decay_t<TSet>>) {
		bind_property_set(setter, name, access);
	}
}

// A method signature is bindable when its return type and every parameter type
// are Variant-marshalable (see the VariantCompatible concept in variant.h).
template <class TRet, class... TArgs>
concept method_signature_bindable =
		VariantCompatible<std::decay_t<TRet>> && (VariantCompatible<std::decay_t<TArgs>> && ...);

template <class T, class TRet, class... TArgs>
inline void ClassDB::bind_method_if_bindable(TRet (T::*method)(TArgs...), std::string_view name, AccessLevel access) {
	if constexpr (method_signature_bindable<TRet, TArgs...>) {
		bind_method(method, name, access);
	}
}

template <class T, class TRet, class... TArgs>
inline void
ClassDB::bind_method_if_bindable(TRet (T::*method)(TArgs...) const, std::string_view name, AccessLevel access) {
	if constexpr (method_signature_bindable<TRet, TArgs...>) {
		bind_method(method, name, access);
	}
}

} //namespace feather
