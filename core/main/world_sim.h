#pragma once

#include "simulation.h"

#include <ecs/components/scene.h>
#include <ecs/ecs_defs.h>
#include <ecs/entity.h>
#include <ecs/query.h>
#include <ecs/world.h>
#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "world_sim.gen.h"
#endif

namespace feather {

// Drives the simulation and owns its scenes. It owns the World but is not a front end for it: entities, component
// types, modules and queries are all World's (ecs/world.h), reached through get_world().
class FEATHER_API WorldSim final : public Simulation {
	FCLASS(singleton);

	World _world;
	Entity _scene_prefab;
	Entity _current_scene;

	std::vector<Entity> _scenes;

public:
	// A timer the world ticks on a fixed interval, for a system that should not run once per frame.
	// Hand it to SystemBuilder::tick_source.
	const EntityId fixed_tick;

	WorldSim();

	void init() override;

	void update(double delta) override;

	// The ECS itself. Entity creation, component types, modules and queries all live there.
	[[nodiscard]] World* get_world() { return &_world; }

	// ---- Scenes ------------------------------------------------------------

	[[nodiscard]] Entity& get_current_scene() { return _current_scene; }

	[[nodiscard]] Entity create_scene(const std::string& name);

	void add_to_scene(Entity entity);

	void set_active_scene(Entity scene);

	// A query restricted to whatever scene is active: ActiveScene sits on the scene entity, so the term is looked for
	// up the hierarchy rather than on the matched entity itself.
	template <class... TComps>
	[[nodiscard]] Query<TComps...> scene_query(const char* name = "") {
		QueryBuilder<TComps...> builder(_world, name);
		builder.template with<ActiveScene>(TraverseFlag::Up);
		return builder.build();
	}
};

} //namespace feather
