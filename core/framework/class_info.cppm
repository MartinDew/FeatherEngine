export module feather.core:framework.class_info;

import std.compat;
import :framework.static_string;
import :framework.callable;
import :framework.variant;

export namespace feather {

// Mirrors the C++ access of a member (or an explicit property attribute); gates
// script/editor access via Variant::get/set/call.
enum class AccessLevel : uint8_t {
	Public = 0,
	Protected,
	Private,
};

// Operations a value type does not support. Distinct from a null hook, which means the operation is trivial.
enum class ValueTypeIllegal : uint8_t {
	None = 0,
	DefaultConstruct = 1 << 0,
	Copy = 1 << 1,
	Move = 1 << 2,
	CopyConstruct = 1 << 3,
	MoveConstruct = 1 << 4,
};

constexpr ValueTypeIllegal operator|(ValueTypeIllegal a, ValueTypeIllegal b) {
	return static_cast<ValueTypeIllegal>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr bool has_flag(ValueTypeIllegal flags, ValueTypeIllegal flag) {
	return (static_cast<uint8_t>(flags) & static_cast<uint8_t>(flag)) != 0;
}

// Lifecycle of a run of `count` instances, callable without naming the type. A null hook that is not marked illegal
// means the operation is trivial, which lets storage move or copy whole runs with memcpy.
struct ValueTypeHooks {
	void (*default_construct)(void* ptr, size_t count) = nullptr;
	void (*destruct)(void* ptr, size_t count) = nullptr;
	void (*copy)(void* dst, const void* src, size_t count) = nullptr;
	void (*move)(void* dst, void* src, size_t count) = nullptr;
	void (*copy_construct)(void* dst, const void* src, size_t count) = nullptr;
	void (*move_construct)(void* dst, void* src, size_t count) = nullptr;
	ValueTypeIllegal illegal = ValueTypeIllegal::None;
};

// copy/move assign into constructed instances, the *_construct hooks into raw storage. Mirrors flecs' trait tests
// (addons/cpp/lifecycle_traits.hpp), so storage treats a type as flecs' C++ API would.
template <class T>
constexpr ValueTypeHooks make_value_type_hooks() {
	ValueTypeHooks hooks;
	if constexpr (!std::is_default_constructible_v<T>) {
		hooks.illegal = hooks.illegal | ValueTypeIllegal::DefaultConstruct;
	}
	else if constexpr (!std::is_trivially_constructible_v<T>) {
		hooks.default_construct = [](void* ptr, size_t count) {
			std::uninitialized_value_construct_n(static_cast<T*>(ptr), count);
		};
	}
	if constexpr (!std::is_trivially_destructible_v<T>) {
		hooks.destruct = [](void* ptr, size_t count) { std::destroy_n(static_cast<T*>(ptr), count); };
	}
	if constexpr (!std::is_copy_assignable_v<T>) {
		hooks.illegal = hooks.illegal | ValueTypeIllegal::Copy;
	}
	else if constexpr (!std::is_trivially_copyable_v<T>) {
		hooks.copy = [](void* dst, const void* src, size_t count) {
			std::copy_n(static_cast<const T*>(src), count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_move_assignable_v<T>) {
		hooks.illegal = hooks.illegal | ValueTypeIllegal::Move;
	}
	else if constexpr (!std::is_trivially_move_assignable_v<T>) {
		hooks.move = [](void* dst, void* src, size_t count) {
			std::move(static_cast<T*>(src), static_cast<T*>(src) + count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_copy_constructible_v<T>) {
		hooks.illegal = hooks.illegal | ValueTypeIllegal::CopyConstruct;
	}
	else if constexpr (!std::is_trivially_copy_constructible_v<T>) {
		hooks.copy_construct = [](void* dst, const void* src, size_t count) {
			std::uninitialized_copy_n(static_cast<const T*>(src), count, static_cast<T*>(dst));
		};
	}
	if constexpr (!std::is_move_constructible_v<T>) {
		hooks.illegal = hooks.illegal | ValueTypeIllegal::MoveConstruct;
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

	// Abstract/singleton classes have a null object_create_func.
	bool is_abstract = false;
	bool is_singleton = false;
	// Value type (FSTRUCT / FCLASS(novtable)): no Reflected base or factory --
	// Property::getter/setter's void* is a raw T*, not a Reflected*.
	bool is_value_type = false;

	// sizeof/alignof of a value type. An empty type still reports sizeof 1, so is_empty tells storage it needs none.
	size_t size = 0;
	size_t alignment = 1;
	bool is_empty = false;

	// Only filled for value types: enough to store instances from this ClassInfo alone.
	ValueTypeHooks value_hooks;

	struct Property {
		StaticString name;
		// variant type to convert to
		VariantType type;

		// Tracked independently so a property can be e.g. publicly readable but
		// privately writable; Variant::get/set only reach Public, *_internal bypasses this.
		AccessLevel getter_access = AccessLevel::Public;
		AccessLevel setter_access = AccessLevel::Public;

		// Function pointers for get/set
		std::function<Variant(void*)> getter; // Takes object pointer, returns Variant
		std::function<void(void*, Variant)> setter; // Takes object pointer and value
	};
	std::vector<Property> properties;

	struct Method {
		StaticString name;
		// Possibly need to store param names later
		AccessLevel access = AccessLevel::Public;
		Callable callable;
	};

	std::vector<Method> methods;

	std::function<Variant()> object_create_func;
};

} //namespace feather
