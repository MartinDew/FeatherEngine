export module feather.core:framework.class_info;

import std.compat;
import :framework.assert;
import :framework.static_string;
import :framework.callable;
import :framework.variant;
export import :framework.reflection_utils;
import :framework.object;

export namespace feather {

// Operations a type does not support. Distinct from a null hook, which means the operation is trivial.
enum class LifecycleIllegal : uint8_t {
	None = 0,
	DefaultConstruct = 1 << 0,
	Copy = 1 << 1,
	Move = 1 << 2,
	CopyConstruct = 1 << 3,
	MoveConstruct = 1 << 4,
	Destruct = 1 << 5,
};

constexpr LifecycleIllegal operator|(LifecycleIllegal a, LifecycleIllegal b) {
	return static_cast<LifecycleIllegal>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr bool has_flag(LifecycleIllegal flags, LifecycleIllegal flag) {
	return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

// Lifecycle of a run of `count` instances, callable without naming the type. A null hook that is not marked illegal
// means the operation is trivial, which lets storage move or copy whole runs with memcpy.
struct LifecycleHooks {
	void (*default_construct)(void* ptr, size_t count) = nullptr;
	void (*destruct)(void* ptr, size_t count) = nullptr;
	void (*copy)(void* dst, const void* src, size_t count) = nullptr;
	void (*move)(void* dst, void* src, size_t count) = nullptr;
	void (*copy_construct)(void* dst, const void* src, size_t count) = nullptr;
	void (*move_construct)(void* dst, void* src, size_t count) = nullptr;
	LifecycleIllegal illegal = LifecycleIllegal::None;
};

// copy/move assign into constructed instances, the *_construct hooks into raw storage. Mirrors flecs' trait tests
// (addons/cpp/lifecycle_traits.hpp), so storage treats a type as flecs' C++ API would.
template <class T>
constexpr LifecycleHooks make_lifecycle_hooks() {
	LifecycleHooks hooks;
	if constexpr (!std::is_default_constructible_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::DefaultConstruct;
	}
	else if constexpr (!std::is_trivially_constructible_v<T>) {
		hooks.default_construct = [](void* ptr, size_t count) {
			std::uninitialized_value_construct_n(static_cast<T*>(ptr), count);
		};
	}
	if constexpr (!std::is_destructible_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::Destruct;
	}
	else if constexpr (!std::is_trivially_destructible_v<T>) {
		hooks.destruct = [](void* ptr, size_t count) { std::destroy_n(static_cast<T*>(ptr), count); };
	}
	if constexpr (!std::is_copy_assignable_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::Copy;
	}
	else if constexpr (!std::is_trivially_copyable_v<T>) {
		hooks.copy = [](void* dst, const void* src, size_t count) {
			std::copy_n(static_cast<const T*>(src), count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_move_assignable_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::Move;
	}
	else if constexpr (!std::is_trivially_move_assignable_v<T>) {
		hooks.move = [](void* dst, void* src, size_t count) {
			std::move(static_cast<T*>(src), static_cast<T*>(src) + count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_copy_constructible_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::CopyConstruct;
	}
	else if constexpr (!std::is_trivially_copy_constructible_v<T>) {
		hooks.copy_construct = [](void* dst, const void* src, size_t count) {
			std::uninitialized_copy_n(static_cast<const T*>(src), count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_move_constructible_v<T>) {
		hooks.illegal = hooks.illegal | LifecycleIllegal::MoveConstruct;
	}
	else if constexpr (!std::is_trivially_move_constructible_v<T>) {
		hooks.move_construct = [](void* dst, void* src, size_t count) {
			std::uninitialized_move_n(static_cast<T*>(src), count, static_cast<T*>(dst));
		};
	}
	return hooks;
}

struct ClassInfo {
	StaticString name = ""_ss;
	StaticString parent = ""_ss;
	std::vector<const ClassInfo*> children;

	// Abstract/singleton classes are never created through reflection.
	bool is_abstract = false;
	bool is_singleton = false;

	// An empty type still reports sizeof 1, so is_empty tells storage it needs none.
	size_t size = 0;
	size_t alignment = 1;
	bool is_empty = false;
	LifecycleHooks lifecycle;

	// Adjusts a pointer to this class into a pointer to its parent subobject; null at a root.
	void* (*to_parent)(void* object) = nullptr;

	struct Property {
		StaticString name;
		// variant type to convert to
		VariantType type;

		// Tracked independently so a property can be e.g. publicly readable but
		// privately writable; Variant::get/set only reach Public, *_internal bypasses this.
		AccessLevel getter_access = AccessLevel::Public;
		AccessLevel setter_access = AccessLevel::Public;

		// Both take a pointer to an instance of the class that declares the property.
		Variant (*getter)(void* object) = nullptr;
		void (*setter)(void* object, const Variant& value) = nullptr;
	};
	std::vector<Property> properties;

	struct Method {
		StaticString name;
		// Possibly need to store param names later
		AccessLevel access = AccessLevel::Public;
		// A non-static method's callable takes the ObjectRef of the declaring class first.
		bool is_static = false;
		Callable callable;
	};

	std::vector<Method> methods;

	bool can_create() const {
		return !has_flag(lifecycle.illegal, LifecycleIllegal::DefaultConstruct) &&
				!has_flag(lifecycle.illegal, LifecycleIllegal::Destruct);
	}
};

enum class ClassKind : uint8_t {
	Auto = 0,
	Abstract,
	Singleton,
};

namespace reflection_detail {

template <class>
struct member_function_traits;

template <class C, class R, class... A>
struct member_function_traits<R (C::*)(A...)> {
	using ret = R;
	using args = std::tuple<A...>;
};
template <class C, class R, class... A>
struct member_function_traits<R (C::*)(A...) const> : member_function_traits<R (C::*)(A...)> {};
template <class C, class R, class... A>
struct member_function_traits<R (C::*)(A...) noexcept> : member_function_traits<R (C::*)(A...)> {};
template <class C, class R, class... A>
struct member_function_traits<R (C::*)(A...) const noexcept> : member_function_traits<R (C::*)(A...)> {};

template <class>
struct data_member_traits;

template <class C, class U>
struct data_member_traits<U C::*> {
	using type = U;
};

template <auto Accessor>
constexpr bool is_data_member = std::is_member_object_pointer_v<decltype(Accessor)>;

// Value type a getter yields or a setter takes, decayed; void when the accessor is null.
template <auto Accessor, bool IsGetter>
struct accessor_value {
	using type = void;
};

template <auto Accessor, bool IsGetter>
	requires(!std::is_null_pointer_v<decltype(Accessor)>)
struct accessor_value<Accessor, IsGetter> {
	static auto pick() {
		if constexpr (is_data_member<Accessor>) {
			return std::type_identity<typename data_member_traits<decltype(Accessor)>::type> {};
		}
		else if constexpr (IsGetter) {
			return std::type_identity<typename member_function_traits<decltype(Accessor)>::ret> {};
		}
		else {
			using args = typename member_function_traits<decltype(Accessor)>::args;
			static_assert(std::tuple_size_v<args> == 1, "A property setter takes exactly one argument");
			return std::type_identity<std::tuple_element_t<0, args>> {};
		}
	}
	using type = std::decay_t<typename decltype(pick())::type>;
};

template <class R, class... A>
constexpr bool signature_bindable = VariantCompatible<std::decay_t<R>> && (VariantCompatible<std::decay_t<A>> && ...);

} // namespace reflection_detail

// Builds the runtime ClassInfo of T from its compile-time description. The description comes from the
// codegen-emitted T::_feather_properties()/_feather_methods(); static reflection would compute the same tuples.
template <is_reflected T>
struct ClassInfoBuilder {
	static constexpr auto properties = [] {
		if constexpr (requires { T::_feather_properties(); }) {
			return T::_feather_properties();
		}
		else {
			return std::tuple<> {};
		}
	}();

	static constexpr auto methods = [] {
		if constexpr (requires { T::_feather_methods(); }) {
			return T::_feather_methods();
		}
		else {
			return std::tuple<> {};
		}
	}();

	static void fill(ClassInfo& info, ClassKind kind = ClassKind::Auto) {
		info.name = T::get_class_static();
		info.parent = T::get_parent_name();
		info.is_abstract = kind == ClassKind::Abstract;
		info.is_singleton = kind == ClassKind::Singleton;
		info.size = sizeof(T);
		info.alignment = alignof(T);
		info.is_empty = std::is_empty_v<T>;
		info.lifecycle = make_lifecycle_hooks<T>();
		if (info.is_abstract || info.is_singleton) {
			info.lifecycle.illegal = info.lifecycle.illegal | LifecycleIllegal::DefaultConstruct;
		}

		if constexpr (requires { typename T::_feather_parent; }) {
			using Parent = typename T::_feather_parent;
			info.to_parent = [](void* object) -> void* { return static_cast<Parent*>(static_cast<T*>(object)); };
		}
		else {
			info.to_parent = nullptr;
		}

		info.properties.clear();
		std::apply([&info](const auto&... desc) { (_add_property(info, desc), ...); }, properties);
		info.methods.clear();
		std::apply([&info](const auto&... desc) { (_add_method(info, desc), ...); }, methods);
	}

private:
	template <auto Getter, auto Setter>
	static void _add_property(ClassInfo& info, const PropertyDesc<Getter, Setter>& desc) {
		using namespace reflection_detail;
		using GetType = typename accessor_value<Getter, true>::type;
		using SetType = typename accessor_value<Setter, false>::type;
		using U = std::conditional_t<std::is_void_v<GetType>, SetType, GetType>;

		// A type Variant can't carry is skipped, so an annotated member never breaks the build.
		if constexpr (VariantCompatible<U>) {
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
	}

	template <auto Function>
	static void _add_method(ClassInfo& info, const MethodDesc<Function>& desc) {
		using namespace reflection_detail;
		using F = decltype(Function);

		if constexpr (std::is_member_function_pointer_v<F>) {
			using Traits = member_function_traits<F>;
			[&]<class... A>(std::type_identity<std::tuple<A...>>) {
				using R = typename Traits::ret;
				static_assert(signature_bindable<R, A...>, "[[method]] signature isn't Variant-marshalable");
				std::function<R(ObjectRef, A...)> func = [](ObjectRef self, A... args) -> R {
					return (static_cast<T*>(self.get())->*Function)(args...);
				};
				info.methods.push_back({ .name = desc.name, .access = desc.access, .callable = Callable { func } });
			}(std::type_identity<typename Traits::args> {});
		}
		else {
			info.methods.push_back(
					{ .name = desc.name, .access = desc.access, .is_static = true, .callable = Callable { Function } }
			);
		}
	}
};

} //namespace feather
