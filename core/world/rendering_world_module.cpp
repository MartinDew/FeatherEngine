#include "rendering_world_module.h"

#include "components/light.h"
#include "main/world_sim.h"
#include "rendering/rendering_server.h"

namespace feather {

void RenderingWorldFeature::_bind_members() {
	ClassDB::bind_static_method(&Type::_load_module, "_import_module");
}

void RenderingWorldFeature::_load_module(WorldSim* sim) {
	sim->get_world()->import <Type>();
}

RenderingWorldFeature::RenderingWorldFeature(World world) {
	std::println("importing module {} ", get_class_static());
	world.module<Type>();

	world.component<MeshInstance>();
	world.component<MaterialInstance>();
	world.component<Light>();

	// todo add iterative way to create buffers
	// world.observer<MeshInstance>()
	// 		.with<const Transform, MaterialInstance>()
	// 		.optional()
	// 		.event(flecs::OnSet)
	// 		.each([](Entity e, const Transform& t, MeshInstance& mi, MaterialInstance* mat) {
	// 			RenderingServer::get()->;
	// 		});

	world.system("Create Render Scene").kind(flecs::PreStore).run([](const flecs::iter& it) {
		world.set<RenderScene>();
	});

	world.system<Transform, MeshInstance, MaterialInstance*>()
			.kind(flecs::PreStore)
			.each([](Entity e, Transform transform, MeshInstance& mesh, MaterialInstance* instance) {
				auto& renderScene = world.get<RenderScene>();
			});
}

} //namespace feather