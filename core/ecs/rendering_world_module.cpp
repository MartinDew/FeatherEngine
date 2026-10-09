#include "rendering_world_module.h"

#include "components/scene.h"
#include "entity.h"
#include "system_builder.h"

#include <rendering/rendering_server.h>
#include <resources/mesh.h>
#include <resources/resource_loader.h>

namespace feather {

void RenderingEcsModule::_begin_render_scene(SystemIterator& it) {
	auto* rs = RenderingServer::get();
	rs->begin_scene_frame();
	rs->set_camera_projection(Projection::create_perspective_fov(90.0f, 16.0f / 9.0f, 0.1f, 1000.0f));
	rs->set_camera_transform({});
}

void RenderingEcsModule::_update_meshes(
		Entity entity,
		Transform transform,
		MeshInstance& mesh,
		MaterialInstance* material
) {
	RenderingServer::get()->add_entity({ transform, mesh.mesh->get_mesh_data(), material ? material->material : nullptr });
}

void RenderingEcsModule::_fill_lights(Entity entity, const Light& light) {
	RenderingServer::get()->add_light(light);
}

void RenderingEcsModule::_commit_render_scene(SystemIterator& it) {
	RenderingServer::get()->commit_scene_frame();
}

void RenderingEcsModule::on_import(World& world) {
	// No terms, so run() rather than each(): the pass has nothing to match but must still fire once a frame.
	world.system<>("Begin Render Scene").phase(SystemPhase::PreStore).run(&_begin_render_scene);

	// ActiveScene sits on the scene entity, so the term is looked for up the hierarchy rather than on the entity
	// itself -- which is what scopes these systems to whichever scene is active.
	world.system<Transform, MeshInstance, MaterialInstance*>("Fill Render Scene")
			.with<ActiveScene>(TraverseFlag::Up)
			.phase(SystemPhase::PreStore)
			.multi_threaded(false)
			.each(&_update_meshes);

	world.system<const Light>("Fill lights")
			.with<ActiveScene>(TraverseFlag::Up)
			.phase(SystemPhase::PreStore)
			.each(&_fill_lights);

	world.system<>("Commit Render Scene").phase(SystemPhase::OnStore).run(&_commit_render_scene);
}

} //namespace feather
