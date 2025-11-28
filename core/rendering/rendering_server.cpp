#include "rendering_server.h"
#include "renderer.h"

#include <main/engine_settings.h>
#include <framework/assert.hpp>
#include <framework/static_string.hpp>

#include <string_view>

namespace feather {

RenderingServer* RenderingServer::_instance = nullptr;

RenderingServer::RenderingServer() {
	fassert(!_instance);

	_instance = this;

	_renderer = renderer_setting.resolve();
}

void RenderingServer::update(double dt) {
	fassert(_renderer.get(), "no renderer set");

	_renderer->_render_scene();
}

} //namespace feather