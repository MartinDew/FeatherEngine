#include "rendering_server.h"

#include <cassert>

namespace feather {
RenderingServer* RenderingServer::_instance = nullptr;

RenderingServer::RenderingServer() {
	assert(_instance);

	_instance = this;
}

} //namespace feather