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

// A container for one component instance on one entity, introspectable through the ClassInfo it resolves on
// assignment: get/set reach the fields through that type's reflection accessors, naming no C++ type.
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

	// Raw storage, for code that knows the layout; the mutable one marks the component changed. Re-resolved per
	// call: adding or removing a component relocates the entity's storage, so a held pointer would dangle.
	[[nodiscard]] const void* data() const;
	[[nodiscard]] void* mutable_data();

	// ---- Property access ---------------------------------------------------

	// Not [[method]]-bound: a bound method's parameters must be Variant-marshalable, and Variant itself is not.
	[[nodiscard]] Variant get(std::string_view property) const;
	bool set(std::string_view property, const Variant& value);
};

} //namespace feather
