module;

#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs_object.gen.h"
#endif

export module feather.core:world.ecs_object;

import std;
import :framework.static_string;
import :framework.reflected;
import :world.ecs;
import :main.class_db;

export namespace feather {

class WorldSim;

// A feature's worth of ECS content. Ecs::import_object builds it once per world and calls on_import inside a scope
// named after the class, so the systems and components it declares are namespaced under it.
class FEATHER_API EcsObject : public Reflected {
	FCLASS();

protected:
	EcsObject() = default;

	static WorldSim* _get_world_sim();

public:
	virtual void on_import(Ecs& ecs) = 0;
};

} //namespace feather
