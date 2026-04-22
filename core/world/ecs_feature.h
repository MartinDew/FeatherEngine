#pragma once

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class WorldSim;

class EcsFeature : public Reflected {
	FCLASS(EcsFeature, Reflected);

protected:
	static void _bind_members() {};
	EcsFeature() = default;

	static WorldSim* _get_world_sim();
};

} //namespace feather