#include "rendering_server.h"
#include "renderer.h"
#include <framework/assert.hpp>

namespace feather {
RenderingServer* RenderingServer::_instance = nullptr;

RenderingServer::RenderingServer() {
	fassert(_instance);

	_instance = this;
}

void RenderingServer::update(double dt) {
	fassert(_renderer.get(), "no renderer set");

	_renderer->_render_scene();
}

} //namespace feather