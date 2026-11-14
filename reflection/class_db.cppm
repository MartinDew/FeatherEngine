module;

#include <framework/singleton_helpers.h>

export module feather.reflection:class_db;

import std.compat;
import feather.framework;
import :callable;
import :class_info;
import :reflection_utils;
import :ftype;
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
	std::unordered_map<StaticString, subclass_delegate_t> _subclass_delegates;

	static void _fire_subclass_delegates(std::string_view class_name);

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(std::string_view name);

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

	inline static const ClassInfo* get_class_info(std::string_view name) { return _get_class_info_internal(name); };

	template <is_reflected T>
	static const ClassInfo* get_class_info() {
		return get_class_info(T::get_class_static());
	}

	static Callable get_static_method(const StaticString& class_name, std::string_view func_name);

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
			: kind == ClassKind::Singleton				 ? "singleton"
			: is_value_type								 ? "value type"
														 : "implementation";
	std::println("Registering class '{}' as {} object", Desc::name, kind_name);

	ClassDB& instance = *get();
	ClassInfo& info = instance._class_infos[Desc::name];
	info.name = Desc::name;
	info.parent = Desc::parent_name;
	info.is_abstract = kind == ClassKind::Abstract;
	info.is_singleton = kind == ClassKind::Singleton;
	info.is_value_type = is_value_type;
	// Singletons are owned by the engine and resolved through T::get(); value types have no Reflected base to
	// hand out.
	if constexpr (!is_value_type && !std::is_abstract_v<T> && std::is_default_constructible_v<T>) {
		if (kind == ClassKind::Auto) {
			info.object_create_func = []() -> Variant { return new T(); };
		}
	}

	if (info.parent != "") {
		instance._class_infos[info.parent].children.emplace_back(&info);
	}

	info.properties.clear();
	std::apply([&info](const auto&... desc) { (_add_property<T>(info, desc), ...); }, Desc::properties);
	info.methods.clear();
	std::apply([&info](const auto&... desc) { (_add_method<T>(info, desc), ...); }, Desc::methods);

	_fire_subclass_delegates(Desc::name);
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
				std::function<R(Reflected*, A...)> func = [](Reflected* self, A... args) -> R {
					return (object_cast<T>(self)->*Function)(args...);
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

} //namespace feather
