#pragma once

#include "ecs_defs.h"
#include "world.h"

#include <framework/class_info.h>
#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <framework/static_string.hpp>
#include <framework/variant.h>

#include <string>

#ifndef FEATHER_REFLECTION_PARSER
#include "component.gen.h"
#endif

namespace feather {

// A container for one component instance living on one entity, reached by type name.
//
// Assigning one fetches the held component's ClassInfo, which is what makes it introspectable: the type's properties,
// their Variant types and their access levels are all readable from get_class_info() without the caller ever naming
// the C++ type. Reading and writing go through those same reflection accessors, so a type the engine was compiled
// against and one a plugin described at runtime are handled identically.
//
// The ClassInfo and the component's id are resolved once, on assignment. The storage pointer deliberately is not:
// adding or removing any component moves the entity to another archetype and relocates its storage, so a cached data
// pointer would silently dangle. data() therefore re-resolves, which after the cached lookup is a single fetch.
class FEATHER_API Component : public Reflected {
	FCLASS();

	World* _world = nullptr;
	EntityId _entity;
	StaticString _type_name = ""_ss;

	// Both filled on assignment, from the type name.
	const ClassInfo* _info = nullptr;
	EntityId _component_id;

	[[nodiscard]] const ClassInfo::Property* _find_property(std::string_view name) const;

public:
	Component() = default;
	Component(World& world, EntityId entity, StaticString type_name);

	[[nodiscard]] StaticString component_name() const { return _type_name; }
	[[nodiscard]] EntityId entity_id() const { return _entity; }

	// What the container knows about the type it holds: properties, their types, their access levels.
	[[nodiscard]] const ClassInfo* get_class_info() const { return _info; }

	explicit operator bool() const { return is_valid(); }

	[[method]] bool is_valid() const;
	[[method]] std::string get_type_name() const;

	// Raw storage, for code that knows the layout. Writing through the mutable one marks the component changed.
	[[nodiscard]] const void* data() const;
	[[nodiscard]] void* mutable_data();

	// ---- Property access ---------------------------------------------------

	// Not [[method]]-bound: a bound method's parameters must be Variant-marshalable, and Variant itself is not.
	[[nodiscard]] Variant get(std::string_view property) const;
	bool set(std::string_view property, const Variant& value);
};

} //namespace feather
