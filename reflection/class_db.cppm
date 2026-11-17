module;

#include <framework/singleton_helpers.h>

export module feather.reflection:class_db;

import std.compat;
import feather.framework;
import :callable;
import :class_info;
import :enum_info;
import :reflection_utils;
import :ftype;
import :object;
import :reflected;
import :variant;

export namespace feather {

enum class ClassKind : uint8_t {
	Auto = 0,
	Abstract,
	Singleton,
};

class FEATHER_REFLECTION_API ClassDB {
	friend Variant;
	FDECLARE_SINGLETON(ClassDB);

public:
	ClassDB();
	~ClassDB();

	using subclass_delegate_t = Delegate<const std::string_view>;

private:
	// Node-based, so a ClassInfo never moves once registered and ClassInfo* stays valid.
	std::unordered_map<StaticString, ClassInfo> _class_infos;
	std::unordered_map<StaticString, EnumInfo> _enum_infos;
	std::unordered_map<StaticString, subclass_delegate_t> _subclass_delegates;
	subclass_delegate_t _class_registered_delegate;

	static void _fire_subclass_delegates(const ClassInfo& info);

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(StaticString name);

	template <is_reflected T>
	static void _register(ClassKind kind);

	template <class T, auto Getter, auto Setter>
	static void _add_property(ClassInfo& info, const PropertyDesc<Getter, Setter>& desc);

	template <class T, auto Function, bool Strict>
	static void _add_method(ClassInfo& info, const MethodDesc<Function, Strict>& desc);

public:
	void print_db();

	// Empties the class registry. Must run before any extension DLL that
	// registered classes gets unloaded -- see Main::~Main().
	static void clear();

	inline static const ClassInfo* get_class_info(StaticString name) { return _get_class_info_internal(name); };

	template <is_reflected T>
	static const ClassInfo* get_class_info() {
		return get_class_info(T::get_class_static());
	}

	static Callable get_static_method(const StaticString& class_name, std::string_view func_name);

	// Register an enum before any class with a property of that type, so the property links to its EnumInfo.
	template <class E>
		requires std::is_enum_v<E>
	static void register_enum() {
		using Desc = FEnum<E>;
		EnumInfo& info = get()->_enum_infos[Desc::name];
		info.name = Desc::name;
		info.underlying_size = sizeof(E);
		info.values.assign(Desc::values.begin(), Desc::values.end());
	}

	static const EnumInfo* get_enum(StaticString name);

	template <class E>
		requires std::is_enum_v<E>
	static const EnumInfo* get_enum() {
		return get_enum(FEnum<E>::name);
	}

	// TODO: runtime registration (C API / scripts) needs a public register_class(ClassInfo), and
	// ClassInfo::Property getter/setter need user data, since only the templated _register<T> can insert today.

	// Abstract, not default-constructible and singleton classes are detected from T itself.
	template <is_reflected T>
	static void register_class() {
		_register<T>(ClassKind::Auto);
	}

	template <is_reflected T>
	static void register_abstract_class() {
		_register<T>(ClassKind::Abstract);
	}

	template <is_reflected T>
	static void register_singleton_class() {
		_register<T>(ClassKind::Singleton);
	}

	// New default-constructed instance owned by the caller, who frees it with destroy(); a Reflected one may instead
	// be deleted. Null when the class is unknown or can't be created.
	static ObjectRef instantiate(StaticString class_name);
	static void destroy(ObjectRef object);

	// Null when the class can't be created or isn't a T.
	template <is_reflected T>
	static std::unique_ptr<T> create_object(StaticString class_name) {
		ObjectRef object = instantiate(class_name);
		T* typed = object.cast<T>();
		if (!typed) {
			destroy(object);
		}
		return std::unique_ptr<T> { typed };
	}

	static Delegate<std::string_view>::id_t on_subclass_registered(
			std::string_view base_class_name,
			const Delegate<std::string_view>::DelegateFuncType& callback
	);

	static void unregister_subclass_delegate(std::string_view base_class_name, Delegate<std::string_view>::id_t id);

	// Fires with the class name after every registration, including classes from extensions loaded later.
	static Delegate<std::string_view>::id_t on_class_registered(
			const Delegate<std::string_view>::DelegateFuncType& callback
	);
	static void unregister_class_delegate(Delegate<std::string_view>::id_t id);

	static std::vector<StaticString> get_class_names();

	static std::vector<StaticString> get_children_names(StaticString object_name, bool exclusive = false);

	static std::string get_children_names_string(StaticString object_name, bool exclusive = false);

	static bool has_parent(StaticString object_name, StaticString parent_name);
};

template <is_reflected T>
void ClassDB::_register(ClassKind kind) {
	using Desc = FType<T>;
	static_assert(std::is_same_v<typename Desc::type, T>, "FType<T>::type must be T");
	constexpr bool is_value_type = is_reflected_value_type<T>;

	if (kind == ClassKind::Auto && !is_value_type) {
		if constexpr (std::is_abstract_v<T> || !std::is_default_constructible_v<T>) {
			kind = ClassKind::Abstract;
		}
		else if constexpr (is_singleton_v<T>) {
			kind = ClassKind::Singleton;
		}
	}
	const char* kind_name = kind == ClassKind::Abstract ? "abstract"
			: kind == ClassKind::Singleton				? "singleton"
			: is_value_type								? "value type"
														: "implementation";
	std::println("Registering class '{}' as {} object", Desc::name, kind_name);

	ClassDB& instance = *get();
	ClassInfo& info = instance._class_infos[Desc::name];
	info.name = Desc::name;
	info.cpp_name = compiler_type_name<T>();
	info.parent = Desc::parent_name;
	info.is_abstract = kind == ClassKind::Abstract;
	info.is_singleton = kind == ClassKind::Singleton;
	info.is_value_type = is_value_type;
	info.size = sizeof(T);
	info.alignment = alignof(T);
	info.is_empty = std::is_empty_v<T>;
	info.lifecycle = make_lifecycle_hooks<T>();
	// Singletons are owned by the engine and resolved through T::get(); neither they nor abstract classes are
	// created through reflection.
	if (kind != ClassKind::Auto) {
		info.lifecycle.illegal = info.lifecycle.illegal | LifecycleIllegal::DefaultConstruct;
	}

	if constexpr (!std::is_void_v<typename Desc::parent>) {
		using Parent = typename Desc::parent;
		info.to_parent = [](void* object) -> void* { return static_cast<Parent*>(static_cast<T*>(object)); };
	}
	else {
		info.to_parent = nullptr;
	}

	info.parent_info = nullptr;
	if (info.parent != ""_ss) {
		// Codegen orders registration parent-first within a module, and modules register bottom-up.
		auto parent = instance._class_infos.find(info.parent);
		fassert(parent != instance._class_infos.end(), "Class registered before its parent");
		info.parent_info = &parent->second;
		// Registering a class again (an extension reloaded without clear()) mustn't list it twice.
		if (!std::ranges::contains(parent->second.children, &info)) {
			parent->second.children.emplace_back(&info);
		}
	}

	info.properties.clear();
	std::apply([&info](const auto&... desc) { (_add_property<T>(info, desc), ...); }, Desc::properties);
	info.methods.clear();
	std::apply([&info](const auto&... desc) { (_add_method<T>(info, desc), ...); }, Desc::methods);

	_fire_subclass_delegates(info);
	instance._class_registered_delegate.execute(info.name.str());
}

template <class T, auto Getter, auto Setter>
void ClassDB::_add_property(ClassInfo& info, const PropertyDesc<Getter, Setter>& desc) {
	using namespace reflection_detail;
	using GetType = typename accessor_value<Getter, true>::type;
	using SetType = typename accessor_value<Setter, false>::type;
	using U = std::conditional_t<std::is_void_v<GetType>, SetType, GetType>;
	static_assert(
			VariantCompatible<U> && (std::is_void_v<SetType> || VariantCompatible<SetType>),
			"Property type isn't Variant-marshalable: add it to Variant or drop [[get]]/[[set]] from the member"
	);

	ClassInfo::Property prop { .name = desc.name,
							   .type = get_variant_type<U>(),
							   .getter_access = desc.getter_access,
							   .setter_access = desc.setter_access };

	if constexpr (!std::is_void_v<GetType>) {
		prop.getter = [](void* object) -> Variant {
			if constexpr (is_data_member<Getter>) {
				return Variant(static_cast<T*>(object)->*Getter);
			}
			else {
				return Variant((static_cast<T*>(object)->*Getter)());
			}
		};
	}
	if constexpr (!std::is_void_v<SetType>) {
		prop.setter = [](void* object, const Variant& value) {
			auto result = value.as<SetType>();
			fassert(result.has_value(), "Property setter called with an incompatible Variant type");
			if constexpr (is_data_member<Setter>) {
				static_cast<T*>(object)->*Setter = std::move(result.value());
			}
			else {
				(static_cast<T*>(object)->*Setter)(std::move(result.value()));
			}
		};
	}
	if constexpr (std::is_enum_v<U> && requires { FEnum<U>::name; }) {
		prop.enum_info = get_enum<U>();
	}
	info.properties.push_back(std::move(prop));
}

template <class T, auto Function, bool Strict>
void ClassDB::_add_method(ClassInfo& info, const MethodDesc<Function, Strict>& desc) {
	using namespace reflection_detail;
	using F = decltype(Function);

	if constexpr (std::is_member_function_pointer_v<F>) {
		using Traits = member_function_traits<F>;
		[&]<class... A>(std::type_identity<std::tuple<A...>>) {
			using R = typename Traits::ret;
			if constexpr (signature_bindable<R, A...>) {
				// self is already adjusted to T by the caller (see Variant::_internal_call).
				std::function<R(ObjectRef, A...)> func = [](ObjectRef self, A... args) -> R {
					return (static_cast<T*>(self.get())->*Function)(args...);
				};
				info.methods.push_back({ .name = desc.name, .access = desc.access, .callable = Callable { func } });
			}
			else {
				static_assert(!Strict, "[[method]] signature isn't Variant-marshalable");
			}
		}(std::type_identity<typename Traits::args> {});
	}
	else {
		info.methods.push_back(
				{ .name = desc.name, .access = desc.access, .is_static = true, .callable = Callable { Function } }
		);
	}
}

template <class T>
	requires(!std::is_const_v<T> && (std::is_base_of_v<Reflected, T> || is_reflected<T>))
ObjectRef::ObjectRef(T& object) {
	if constexpr (std::is_base_of_v<Reflected, T>) {
		*this = static_cast<Reflected&>(object)._get_object_ref();
	}
	else {
		_ptr = &object;
		_info = ClassDB::get_class_info<T>();
	}
}

template <is_reflected T>
T* ObjectRef::cast() const {
	using U = std::remove_const_t<T>;
	if constexpr (std::is_base_of_v<Reflected, U>) {
		return object_cast<T>(_reflected);
	}
	else {
		return static_cast<T*>(cast_to(ClassDB::get_class_info<U>()));
	}
}

} //namespace feather
