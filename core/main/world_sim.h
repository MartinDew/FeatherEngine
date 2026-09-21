#pragma once

#include "simulation.h"

#include <ecs/components/scene.h>
#include <ecs/ecs_defs.h>
#include <ecs/entity.h>
#include <ecs/query.h>
#include <ecs/world.h>
#include <framework/delegate.h>
#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "world_sim.gen.h"
#endif

namespace feather {

class FEATHER_API WorldSim final : public Simulation {
	FCLASS(singleton);

	World _world;
	Entity _scene_prefab;
	Entity _current_scene;

	std::vector<Entity> _scenes;

	ClassDB::subclass_delegate_t::id_t _subclass_delegate_id = -1;

protected:
	template <std::derived_from<class EcsModule> T>
	void _import_feature() {
		_world.import_module<T>();
	}

public:
	// A timer the world ticks on a fixed interval, for a system that should not run once per frame.
	// Hand it to SystemBuilder::tick_source.
	const EntityId fixed_tick;

	WorldSim();
	~WorldSim() override;

	void init() override;

	void update(double delta) override;

	[[nodiscard]] Entity& get_current_scene() { return _current_scene; }

	[[nodiscard]] Entity create_scene(const std::string& name);

	[[nodiscard]] Entity create_entity(const std::string& name = "");
	[[nodiscard]] Entity create_entity(const Entity& parent_entity, const std::string& name = "");

	[[nodiscard]] World* get_world() { return &_world; }

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
