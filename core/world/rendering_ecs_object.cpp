module feather.core;

import std;

namespace feather {

namespace {

void begin_render_scene(SystemIterator&) {
	auto* rs = RenderingServer::get();
	rs->begin_scene_frame();
	rs->set_camera_projection(Projection::create_perspective_fov(90.0f, 16.0f / 9.0f, 0.1f, 1000.0f));
	rs->set_camera_transform({});
}

void update_meshes(Entity, const Transform& transform, const MeshInstance& mesh, const MaterialInstance* mat) {
	RenderingServer::get()->add_entity({ transform, mesh.mesh->get_mesh_data(), mat ? mat->material : nullptr });
}

} //namespace

void RenderingEcsObject::on_import(Ecs& ecs) {
	ecs.system("Begin Render Scene").phase(SystemPhase::PreStore).run(&begin_render_scene);

	ecs.system<const Transform, const MeshInstance, const MaterialInstance*>("Fill Render Scene")
			.with<ActiveScene>(TraverseFlag::Up)
			.phase(SystemPhase::PreStore)
			.multi_threaded(false)
			.each(&update_meshes);

	ecs.system<const Light>("Fill lights")
			.phase(SystemPhase::PreStore)
			.with<ActiveScene>(TraverseFlag::Up)
			.each([](Entity, const Light& light) { RenderingServer::get()->add_light(light); });

	ecs.system("Commit Render Scene").phase(SystemPhase::OnStore).run([](SystemIterator&) {
		RenderingServer::get()->commit_scene_frame();
	});
}

} //namespace feather
