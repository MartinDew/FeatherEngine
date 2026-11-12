export module feather.reflection:object_access;

import std.compat;
import feather.framework;
import :reflected;
import :variant;

export namespace feather {

// How Variant reaches an object's properties and methods. Implemented by ClassDB (core), which
// sits above reflection, so Variant doesn't need to know about it.
struct ObjectAccess {
	virtual ~ObjectAccess() = default;

	// enforce_public: script-facing access only reaches AccessLevel::Public members.
	virtual Variant object_get(Reflected* obj, std::string_view key, bool enforce_public) const = 0;
	virtual void object_set(Reflected* obj, std::string_view key, const Variant& value, bool enforce_public) const = 0;
	virtual Variant
	object_call(Reflected* obj, std::string_view method, std::span<Variant> args, bool enforce_public) const = 0;
};

// Set by core when a ClassDB exists. While null, Variant object access returns NIL.
FEATHER_REFLECTION_API void set_object_access(const ObjectAccess* access);
FEATHER_REFLECTION_API const ObjectAccess* get_object_access();

} // namespace feather
