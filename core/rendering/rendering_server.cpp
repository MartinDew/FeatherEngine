#include "rendering_server.h"
#include "renderer.h"
#include <framework/assert.hpp>

namespace feather {
RenderingServer* RenderingServer::_instance = nullptr;

RenderingServer::RenderingServer() {
	fassert(_instance);

	_instance = this;
}

void update(double dt) {
}

} //namespace feather