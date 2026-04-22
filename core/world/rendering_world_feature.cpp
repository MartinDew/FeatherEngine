#include "rendering_world_feature.h"

#include <main/world_sim.h>
#include <rendering/rendering_server.h>
#include <resources/mesh.h>
#include <resources/resource_loader.h>
#include <world/components/light.h>

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

	world.component<MeshInstance>("MeshInstance");
	world.component<MaterialInstance>("MaterialInstance");
	world.component<Light>("Light");
	world.system("Create Render Scene").kind(flecs::PreStore).write<RenderScene>().run([](const flecs::iter& it) {
		it.world().set<RenderScene>({});
	});

	world.system<Transform, MeshInstance, MaterialInstance*>("Fill Render Scene")
			.read<RenderScene>()
			.kind(flecs::PreStore)
			.multi_threaded(false)
			.each([](Entity e, Transform transform, MeshInstance& mesh, MaterialInstance* mat) {
				RenderScene& renderScene = e.world().get_mut<RenderScene>();
				renderScene.add_entity({ transform, mesh.mesh->get_mesh_data(), mat ? mat->material : nullptr });
			});

	world.system("Load Render Scene").read<RenderScene>().kind(flecs::OnStore).run([](const flecs::iter& it) {
		const RenderScene& rs = it.world().get<RenderScene>();
		RenderingServer::get()->set_render_capture(rs);
	});
}

} //namespace feather