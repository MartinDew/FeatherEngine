#include "rendering_server.h"
#include "renderer.h"

#include <main/engine_settings.h>
#include <framework/assert.hpp>
#include <framework/static_string.hpp>

#include <string_view>

#if vex_renderer_ENABLED
#include "modules/vex_renderer/vex_renderer.h"
#endif
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

void RenderingServer::use_renderer(StaticString name) {
	switch (name) {
#ifdef vex_renderer_ENABLED
	case "vex"_ss:
		use_renderer<VexRenderer>();
		break;
#endif
	}
}

RenderingServer* RenderingServer::get() {
	fassert(_instance, "instance not yet initialized for RenderingServer");
	return _instance;
};

} //namespace feather