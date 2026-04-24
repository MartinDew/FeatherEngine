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

inline void _create_render_scene(const flecs::iter& it) {
	static size_t frame_num = 0;

	RenderScene scene { frame_num++ };
	auto camera_projection = Projection::create_perspective_fov(90.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

	scene.set_camera_transform({});
	scene.set_camera_projection(camera_projection);

	it.world().add<RenderScene>();
	it.world().set(scene);
}

RenderingWorldFeature::RenderingWorldFeature(World world) {
	std::println("importing module {} ", get_class_static());
	world.module<Type>();

	world.component<MeshInstance>("MeshInstance");
	world.component<MaterialInstance>("MaterialInstance");
	world.component<Light>("Light");
	auto render_scene_sys =
			world.system("Create Render Scene").kind(flecs::PreStore).write<RenderScene>().run(&_create_render_scene);

	world.system<Transform, MeshInstance, MaterialInstance*>("Fill Render Scene")
			.kind(flecs::PreStore)
			.read<RenderScene>()
			.multi_threaded(false)
			.each([](Entity e, Transform transform, MeshInstance& mesh, MaterialInstance* mat) {
				RenderScene& renderScene = e.world().get_mut<RenderScene>();
				renderScene.add_entity({ transform, mesh.mesh->get_mesh_data(), mat ? mat->material : nullptr });
			});

	world.system<const Light>("Fill lights")
			.kind(flecs::PreStore)
			.read<RenderScene>()
			.each([](Entity e, const Light& light) {
				RenderScene& renderScene = e.world().get_mut<RenderScene>();
				renderScene.add_light(light);
			});

	world.system("Load Render Scene").read<RenderScene>().kind(flecs::OnStore).run([](const flecs::iter& it) {
		const RenderScene& rs = it.world().get<RenderScene>();
		RenderingServer::get()->set_render_capture(rs);
	});
}

} //namespace feather