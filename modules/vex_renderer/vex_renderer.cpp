#include "vex_renderer.h"

#include "main/engine.h"
#include <math/math_defs.h>

namespace feather {

static std::array<Vector3, 8> example_cube = {
	Vector3(-1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f, 1.0f, -1.0f),
	Vector3(-1.0f, 1.0f, -1.0f),
	Vector3(-1.0f, -1.0f, 1.0f),
	Vector3(1.0f, -1.0f, 1.0f),
	Vector3(1.0f, 1.0f, 1.0f),
	Vector3(-1.0f, 1.0f, 1.0f),
};

VexRenderer::VexRenderer() {
}

void VexRenderer::_render_scene() {
}

void VexRenderer::_on_resize() {
	
}
} //namespace feather