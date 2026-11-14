export module feather.reflection:class_info;

import std.compat;
import feather.framework;
import :callable;
import :ftype;
import :variant;

export namespace feather {

struct FEATHER_REFLECTION_API ClassInfo {
	StaticString name = ""_ss;
	StaticString parent = ""_ss;
	std::vector<const ClassInfo*> children;

	// Abstract/singleton classes have a null object_create_func.
	bool is_abstract = false;
	bool is_singleton = false;
	// Value type (FSTRUCT / FCLASS(novtable)): no Reflected base or factory --
	// Property::getter/setter's void* is a raw T*, not a Reflected*.
	bool is_value_type = false;

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
		bool is_static = false;
		Callable callable;
	};

	std::vector<Method> methods;

	std::function<Variant()> object_create_func;
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
