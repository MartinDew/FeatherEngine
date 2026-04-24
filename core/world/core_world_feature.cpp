#include "core_world_feature.h"

#include "components/scene.h"
#include "main/world_sim.h"

namespace feather {

void CoreWorldFeature::_bind_members() {
}

CoreWorldFeature::CoreWorldFeature(World world) {
	world.component<Scene>();
	world.component<InScene>("InScene");

	world.observer<const Scene>()
			.event(Ecs::OnSet)
			.event(Ecs::OnAdd)
			.write<InScene>()
			.each([sim = WorldSim::get()](Entity e, const Scene& scene) {
				const Scene& current_scene = sim->get_current_scene().get<const Scene>();
				if (current_scene == scene)
					e.add<InScene>();
				else
					e.remove<InScene>();
			});
}

} //namespace feather