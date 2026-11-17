export module feather.reflection:ftype;

import std.compat;
import feather.framework;

export namespace feather {

// Mirrors the C++ access of a member (or an explicit property attribute); gates
// script/editor access via Variant::get/set/call.
enum class AccessLevel : uint8_t {
	Public = 0,
	Protected,
	Private,
};

// Compile-time description of one property. Getter and Setter are each nullptr, a data member pointer, or an
// accessor member function pointer.
template <auto Getter, auto Setter>
struct PropertyDesc {
	static constexpr auto getter = Getter;
	static constexpr auto setter = Setter;

	StaticString name;
	AccessLevel getter_access = AccessLevel::Public;
	AccessLevel setter_access = AccessLevel::Public;
};

// Compile-time description of one method: a member function pointer or a static function pointer. A non-strict
// method is skipped when its signature isn't Variant-marshalable; a strict one fails the build.
template <auto Function, bool Strict = true>
struct MethodDesc {
	static constexpr auto function = Function;
	static constexpr bool strict = Strict;

	StaticString name;
	AccessLevel access = AccessLevel::Public;
};

// Compile-time description of a reflected type, specialized by codegen (see FType<Reflected> for the shape).
// Left undefined, so registering a type without a description is a compile error.
template <class T>
struct FType;

} // namespace feather
