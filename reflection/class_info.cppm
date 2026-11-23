export module feather.reflection:class_info;

import std.compat;
import feather.framework;
import :callable;
import :enum_info;
import :reflected;
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
	// The type's operator==; null when it has none.
	bool (*equals)(const void* a, const void* b) = nullptr;
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
	if constexpr (std::equality_comparable<T>) {
		hooks.equals = [](const void* a, const void* b) {
			return *static_cast<const T*>(a) == *static_cast<const T*>(b);
		};
	}
	return hooks;
}

namespace reflection_detail {

// T's name as the compiler spells it in a function signature, which needs no RTTI. The branches and trimming follow
// flecs_cpp.h's ECS_FUNC_NAME, so the name matches the identity flecs derives for T on each compiler.
template <class T>
struct CompilerTypeName {
	static constexpr std::string_view slice() {
#if defined(__clang__)
		constexpr std::string_view signature = __PRETTY_FUNCTION__; // "... [T = feather::Light]"
		constexpr std::size_t start = signature.find("T = ") + 4;
		return signature.substr(start, signature.rfind(']') - start);
#elif defined(__GNUC__)
		constexpr std::string_view signature = __PRETTY_FUNCTION__; // "... [with T = feather::Light@module; ...]"
		constexpr std::size_t start = signature.find("T = ") + 4;
		return signature.substr(start, signature.find_first_of(";]", start) - start);
#elif defined(_MSC_VER)
		constexpr std::string_view signature =
				__FUNCSIG__; // "... CompilerTypeName<struct feather::Light>::slice(void)"
		constexpr std::string_view open = "CompilerTypeName<";
		constexpr std::size_t start = signature.find(open) + open.size();
		std::string_view name = signature.substr(start, signature.rfind(">::slice(void)") - start);
		constexpr std::array<std::string_view, 3> prefixes { "struct ", "class ", "enum " };
		for (std::string_view prefix : prefixes) {
			if (name.starts_with(prefix)) {
				name.remove_prefix(prefix.size());
			}
		}
		return name;
#else
#error "No compiler type name for this compiler"
#endif
	}

	// Owned and NUL-terminated, so the name outlives the signature it was cut from.
	static constexpr auto storage = [] {
		std::array<char, slice().size() + 1> chars {};
		std::ranges::copy(slice(), chars.begin());
		return chars;
	}();
};

} // namespace reflection_detail

// T's fully qualified name as the compiler spells it, e.g. "feather::Light".
template <class T>
constexpr std::string_view compiler_type_name() {
	using Name = reflection_detail::CompilerTypeName<T>;
	return { Name::storage.data(), Name::storage.size() - 1 };
}

struct FEATHER_REFLECTION_API ClassInfo {
	StaticString name = ""_ss;
	// The C++ type's fully qualified name as the compiler spells it; see compiler_type_name().
	StaticString cpp_name = ""_ss;
	// The build module that declares the type (FMODULE_NAME); its ECS component lives under feather.<module_name>.
	StaticString module_name = ""_ss;
	StaticString parent = ""_ss;
	std::vector<const ClassInfo*> children;
	// The FCLASS(...)/FSTRUCT(...) modifier names, opaque to reflection.
	std::vector<StaticString> modifiers;

	// Abstract/singleton classes are never created through reflection.
	bool is_abstract = false;
	bool is_singleton = false;
	// Not derived from Reflected: stored by value, with no vtable.
	bool is_value_type = false;

	// An empty type still reports sizeof 1, so is_empty tells storage it needs none.
	size_t size = 0;
	size_t alignment = 1;
	bool is_empty = false;
	LifecycleHooks lifecycle;

	// Adjusts a Reflected pointer to this class's subobject of the same object; null for a value type.
	void* (*from_reflected)(Reflected* object) = nullptr;
	// A new default-constructed instance, freed with delete; null unless the class can be created.
	Reflected* (*create_reflected)() = nullptr;
	// The registered parent, set by ClassDB at registration; null at a root.
	const ClassInfo* parent_info = nullptr;

	struct Property {
		StaticString name;
		// variant type to convert to
		VariantType type;
		// Set when the property is a registered enum; its Variant is still the INT value.
		const EnumInfo* enum_info = nullptr;

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
		bool is_static = false;
		uint8_t param_count = 0;
		// Takes a pointer to an instance of the declaring class, like the property accessors; null for a static method.
		Variant (*invoke)(void* self, std::span<Variant> args) = nullptr;
	};

	std::vector<Method> methods;

	// Visits (class, instance adjusted to that class) from this class, the instance's own, up to the root; stops at
	// the first level visit returns true for. An object's instance is its Reflected*, a value's is the value itself.
	template <class TVisit>
	bool walk(void* instance, TVisit&& visit) const {
		for (const ClassInfo* info = this; info && instance; info = info->parent_info) {
			void* self = info->from_reflected ? info->from_reflected(static_cast<Reflected*>(instance)) : instance;
			if (visit(*info, self)) {
				return true;
			}
		}
		return false;
	}

	bool has_modifier(StaticString modifier) const { return std::ranges::contains(modifiers, modifier); }

	bool can_create() const {
		return !has_flag(lifecycle.illegal, LifecycleIllegal::DefaultConstruct) &&
				!has_flag(lifecycle.illegal, LifecycleIllegal::Destruct);
	}
	// A copy of an existing instance needs no default constructor.
	bool can_copy() const {
		return !has_flag(lifecycle.illegal, LifecycleIllegal::CopyConstruct) &&
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

} // namespace reflection_detail

} //namespace feather
