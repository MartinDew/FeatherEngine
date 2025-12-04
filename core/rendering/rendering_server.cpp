#include "rendering_server.h"
#include "renderer.h"

#include <framework/assert.h>
#include <main/class_db.h>
#include <main/engine_settings.h>
#include <framework/static_string.hpp>

#include <any>
#include <string_view>

namespace feather {

RenderingServer* RenderingServer::_instance = nullptr;

RenderingServer::RenderingServer() {
	fassert(!_instance);

	_instance = this;
}

void RenderingServer::update(double dt) {
	fassert(_renderer.get(), "no renderer set");

	_renderer->_render_scene();
}

void RenderingServer::use_renderer(std::string_view name) { _renderer = ClassDB::create_object<Renderer>(name); }

RenderingServer* RenderingServer::get() {
	fassert(_instance, "instance not yet initialized for RenderingServer");
	return _instance;
};

} //namespace feather