#pragma once

#include "ecs_defs.h"
#include "world.h"

#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <framework/static_string.hpp>
#include <framework/variant.h>

#include <string>

#ifndef FEATHER_REFLECTION_PARSER
#include "component_handle.gen.h"
#endif

namespace feather {

// A handle to one component instance on one entity, reached by type name.
//
// Reading and writing go through the component's reflection accessors, so this
// works for a type the engine was compiled against and one a plugin described
// at runtime without telling the two apart.
//
// The typed accessors exist because a bound method's parameters have to be
// Variant-marshalable and Variant itself is not; get() and set() are the same
// operations for engine code, which can pass a Variant directly.
class ComponentHandle : public Reflected {
	FCLASS();

	World* _world = nullptr;
	Ecs::entity_t _entity = 0;
	StaticString _component = ""_ss;

public:
	ComponentHandle() = default;
	ComponentHandle(World& world, Ecs::entity_t entity, StaticString component) :
		_world(&world), _entity(entity), _component(component) {}

	[[nodiscard]] StaticString component_name() const { return _component; }
	[[nodiscard]] Ecs::entity_t entity_id() const { return _entity; }

	explicit operator bool() const { return is_valid(); }

	[[method]] bool is_valid() const;
	[[method]] std::string get_type_name() const;

	// Raw storage, for code that knows the layout. Writing through the mutable
	// one marks the component changed.
	[[nodiscard]] const void* data() const;
	[[nodiscard]] void* mutable_data();

	// ---- Property access ---------------------------------------------------

	[[nodiscard]] Variant get(std::string_view property) const;
	bool set(std::string_view property, const Variant& value);

	[[method]] bool get_bool(std::string property) const;
	[[method]] int get_int(std::string property) const;
	[[method]] real_t get_float(std::string property) const;
	[[method]] Vector2 get_vector2(std::string property) const;
	[[method]] Vector3 get_vector3(std::string property) const;
	[[method]] Color get_color(std::string property) const;

	[[method]] bool set_bool(std::string property, bool value);
	[[method]] bool set_int(std::string property, int value);
	[[method]] bool set_float(std::string property, real_t value);
	[[method]] bool set_vector2(std::string property, Vector2 value);
	[[method]] bool set_vector3(std::string property, Vector3 value);
	[[method]] bool set_color(std::string property, Color value);
};

} //namespace feather
