#pragma once

#include "system_builder.h"
#include "world.h"

#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs_module.gen.h"
#endif

namespace feather {

class WorldSim;

// A feature's worth of ECS content: the systems it runs, and any component type that has no C++ class of its own to
// declare it. World finds every subclass through ClassDB and imports it (World::import_modules), which constructs it
// with the world -- so a module's constructor is where it declares what it owns.
//
// Component types need nothing here: deriving from IComponent is enough, and World picks them up from ClassDB.
class FEATHER_API EcsModule : public Reflected {
	FCLASS();

protected:
	EcsModule() = default;

	static WorldSim* _get_world_sim();

	// The only way to declare a system. Protected on purpose: a system belongs to the module that declares it, and
	// the [[system]] attribute holds a method to the same rule (codegen rejects it elsewhere).
	template <class... TComps>
	static SystemBuilder<TComps...> system(World& world, const char* name) {
		return SystemBuilder<TComps...>(world, name);
	}
};

} //namespace feather
