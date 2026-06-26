#pragma once

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class WorldSim;

class EcsModule : public Reflected {
	FCLASS(EcsModule, Reflected);

protected:
	static void _bind_members() {};
	EcsModule() = default;

	static WorldSim* _get_world_sim();
};

} //namespace feather