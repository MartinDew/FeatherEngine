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
	_scene_prefab = _world.prefab("Scene");
	auto scene = create_scene("new scene");
	fassert(scene.is_valid());
	set_active_scene(scene);

	auto children = ClassDB::get_children_names("EcsFeature");
	for (auto& child : children) {
		ClassDB::get_static_method(child, "_import_module").call(this);
	}
}

WorldSim::~WorldSim() = default;

void WorldSim::update(double delta) {
	bool result = _world.progress(/*delta*/);
}

Entity WorldSim::create_scene(std::string name) const {
	Scene s { { name } };
	return _world.prefab(name.c_str()).is_a(_scene_prefab).set<Scene>(s);
}

Entity WorldSim::add_entity(std::string name) const {
	return add_entity(std::move(_current_scene), name);
}

Entity WorldSim::add_entity(const Entity& scene, std::string name) const {
	return _world.entity(name.c_str()).child_of(scene);
}

void WorldSim::add_to_scene(Entity entity) const {
	entity.child_of(_current_scene);
}

void WorldSim::_tag_descendants_in_scene(Entity parent) const {
	parent.children([&](flecs::entity child) {
		child.add<InScene>();
		_tag_descendants_in_scene(child);
	});
}

bool WorldSim::_is_in_scene(flecs::entity e, Entity scene) const {
	flecs::entity current = e;
	while (current.is_valid()) {
		if (current == scene)
			return true;
		current = current.parent();
	}
	return false;
}

void WorldSim::set_active_scene(Entity scene) {
	fassert(scene.is_a(_scene_prefab), "Given scene isn't a scene instance");

	_world.remove_all<InScene>(); // no const here — remove_all needs mutable

	scene.add<InScene>();
	_tag_descendants_in_scene(scene);

	_current_scene = scene;

	if (_scene_observer) {
		_scene_observer.destruct();
	}

	// Instead of observer, use a system that propagates InScene downward.
	// Any entity whose PARENT has InScene but itself doesn't -> add it.
	_scene_observer =
			_world.observer<>().with(flecs::ChildOf, flecs::Wildcard).event(flecs::OnAdd).each([this](flecs::entity e) {
				flecs::entity p = e.parent();
				if (p.is_valid() && p.has<InScene>()) {
					e.add<InScene>();
				}
			});
}

} //namespace feather