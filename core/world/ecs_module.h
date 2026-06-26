#pragma once

#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <world/ecs_defs.h>

namespace feather {
class World;

class EcsModule : public Reflected {
	FCLASS(EcsModule, Reflected);

protected:
	static void _bind_members() {};
	EcsModule() = default;

	static World* _get_world();
};

} //namespace feather
