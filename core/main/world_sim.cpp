module;

#include <framework/singleton_helpers.h>

module feather.core;

import std;

namespace feather {

FSINGLETON_INSTANCE(WorldSim);

WorldSim::WorldSim() : fixed_tick { _ecs.create_timer(Engine::simulation_time) } {
	FSINGLETON_CONSTRUCT_INSTANCE()
#if BETA
	_ecs.enable_rest_api();
#endif
}

WorldSim::~WorldSim() {
	ClassDB::unregister_subclass_delegate(EcsObject::get_class_static(), _subclass_delegate_id);
}

void WorldSim::init() {
	// Every reflected Component registers before any EcsObject imports
	register_core_components(_ecs);

	// Not "Scene": that name is the Scene component's.
	_scene_prefab = _ecs.prefab("ScenePrefab");
	auto scene = create_scene("new scene");
	fassert(scene.is_valid());
	set_active_scene(scene);

	// Subscribed here, not in the ctor (which runs before index_project()):
	// registering a class fires this immediately, so a DLL's EcsObject would otherwise import reentrantly, before the
	// world above exists.
	_subclass_delegate_id = ClassDB::on_subclass_registered(
			EcsObject::get_class_static(), [world_sim = this](std::string_view class_name) {
				if (world_sim) {
					ClassDB::get_static_method(class_name, "_import_object").call(world_sim);
				}
			}
	);

	// Picks up EcsObject subclasses from core, from built-in modules, and --
	// because this runs after index_project() -- from loaded project DLLs.
	auto children = ClassDB::get_children_names(EcsObject::get_class_static());
	for (auto& child : children) {
		ClassDB::get_static_method(child, "_import_object").call(this);
	}
}

void WorldSim::update(double delta) {
	_ecs.progress();
}

Entity WorldSim::create_scene(const std::string& name) {
	return _ecs.entity(name).is_a(_scene_prefab).set(Scene { std::string_view(name) });
}

Entity WorldSim::create_entity(const std::string& name) {
	return _ecs.entity(name);
}

Entity WorldSim::create_entity(const Entity& parent_entity, const std::string& name) {
	return _ecs.entity(name).child_of(parent_entity);
}

void WorldSim::add_to_scene(Entity entity) {
	entity.child_of(_current_scene);
}

bool WorldSim::_is_in_scene(Entity e, Entity scene) const {
	return e.is_descendant_of(scene);
}

void WorldSim::set_active_scene(Entity scene) {
	fassert(scene.is_instance_of(_scene_prefab), "Given scene isn't a scene instance");

	// Clear old active scene marker
	_ecs.remove_all(ActiveScene::get_class_static());

	scene.add<ActiveScene>();
	_current_scene = scene;
}

} //namespace feather
