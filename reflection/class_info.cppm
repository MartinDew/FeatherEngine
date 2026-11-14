export module feather.reflection:class_info;

import std.compat;
import feather.framework;
import :callable;
import :ftype;
import :variant;

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
	else if constexpr (!std::is_trivially_default_constructible_v<T>) {
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
	else if constexpr (!std::is_trivially_copy_assignable_v<T>) {
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

struct FEATHER_REFLECTION_API ClassInfo {
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

} //namespace feather
