#include "render_scene.h"
#include <world/components/scene.h>

#include <world/components/light.h>

namespace feather {

RenderScene::RenderScene(size_t frame_index) : _frame_index(frame_index) {
}

RenderScene::RenderScene(const RenderScene&) = default;

RenderScene& RenderScene::operator=(const RenderScene&) = default;
RenderScene::RenderScene(RenderScene&&) noexcept = default;
RenderScene& RenderScene::operator=(RenderScene&&) noexcept = default;

const Transform& RenderScene::get_camera_transform() const noexcept {
	return _camera_transform;
}

void RenderScene::set_camera_transform(const Transform& transform) {
	_camera_transform = transform;
}

const Projection& RenderScene::get_camera_projection() const noexcept {
	return _camera_projection;
}

void RenderScene::set_camera_projection(const Projection& projection) {
	_camera_projection = projection;
}

void RenderScene::add_entity(const EntityRender& entity) {
	_entities.push_back(entity);
}

void RenderScene::reserve_entities(size_t count) {
	_entities.reserve(count);
}

const CowVector<RenderScene::EntityRender>& RenderScene::get_entities() const noexcept {
	return _entities;
}

size_t RenderScene::get_entity_count() const noexcept {
	return _entities.size();
}

uint64_t RenderScene::get_frame_index() const noexcept {
	return _frame_index;
}

const RenderScene::EnvironmentSettings& RenderScene::get_environment() const noexcept {
	return _environment;
}

void RenderScene::set_environment(const EnvironmentSettings& env) {
	_environment = env;
}

void RenderScene::_bind_members() {
}

void RenderScene::add_light(const Light& light) {
	_lights.push_back(light);
}

void RenderScene::reserve_lights(size_t count) {
	_lights.reserve(count);
}

const CowVector<Light>& RenderScene::get_lights() const noexcept {
	return _lights;
}

size_t RenderScene::get_light_count() const noexcept {
	return _lights.size();
}

void RenderScene::clear() {
	_entities.clear();
	_lights.clear();
	_frame_index++;
}

} //namespace feather