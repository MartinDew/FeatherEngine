#include "world_sim.h"

#include "engine.h"
#include <world/components/scene.h>
#include <world/register_core_features.h>
#include <framework/static_string.hpp>

namespace feather {

FSINGLETON_INSTANCE(WorldSim);

void WorldSim::_bind_members() {
	// ClassDB::bind_property(&Type::fixed_tick, "fixed_tick", VariantType::INT);
}

WorldSim::WorldSim() : fixed_tick { _world.timer().interval(Engine::simulation_time) } {
	FSINGLETON_CONSTRUCT_INSTANCE()

	register_core_ecs_features(_world);

	_prefabs["new_scene"_ss] = _world.prefab("new_scene").emplace<Scene>("new_scene"_ss);
	_scenes.push_back(create_scene());
	_current_scene = _scenes[0];

	auto children = ClassDB::get_children_names("EcsFeature");
	for (auto& child : children) {
		ClassDB::get_static_method(child, "_import_module").call(this);
	}
}

WorldSim::~WorldSim() = default;

void WorldSim::update(double delta) {
	bool result = _world.progress(/*delta*/);
}

Entity WorldSim::create_scene() const {
	return _world.entity().is_a(_prefabs.at("new_scene"_ss));
}

Entity WorldSim::add_entity(const Entity& scene) const {
	return _world.entity().add(scene);
}

void WorldSim::add_to_scene(Entity entity) const {
	entity.child_of(_current_scene);
}

} //namespace feather