#include "world_sim.h"

#include "engine.h"
#include "world/ecs_module.h"
#include <world/ecs_defs.h>
#include <world/register_ecs_types.h>
#include <framework/static_string.hpp>

namespace feather {

FSINGLETON_INSTANCE(WorldSim);

void WorldSim::_bind_members() {
}

WorldSim::WorldSim() {
	FSINGLETON_CONSTRUCT_INSTANCE()
#if BETA
	_world.set<Ecs::Rest>({});
#endif

	register_ecs_types(_world);
	_world.init_scenes();

	auto scene = _world.create_scene("new scene");
	fassert(scene.is_valid());
	_world.set_active_scene(scene);
	_world.set_fixed_tick_interval(Engine::simulation_time);

	auto children = ClassDB::get_children_names<EcsModule>();
	for (auto& child : children) {
		ClassDB::get_static_method(child, "_import_module").call(this);
	}
}

WorldSim::~WorldSim() = default;

void WorldSim::update(double delta) {
	_world.progress(delta);
}

} //namespace feather