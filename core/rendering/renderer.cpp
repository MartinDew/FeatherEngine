#include "renderer.h"

#include "framework/functions.h"
#include "framework/reflection_macros.h"
#include "main/class_db.h"
#include "main/engine.h"
#include "main/window.h"
#include "rendering/triangle_mesh.h"

namespace feather {

namespace {

constexpr std::array<Vertex, 8> example_cube_vertices {
	// Front face
	Vertex { -0.5f, -0.5f, 0.5f, 0, 0 }, // 0: bottom-left
	Vertex { 0.5f, -0.5f, 0.5f, 1, 0 }, // 1: bottom-right
	Vertex { 0.5f, 0.5f, 0.5f, 1, 1 }, // 2: top-right
	Vertex { -0.5f, 0.5f, 0.5f, 0, 1 }, // 3: top-left
	// Back face
	Vertex { -0.5f, -0.5f, -0.5f, 1, 0 }, // 4: bottom-left
	Vertex { 0.5f, -0.5f, -0.5f, 0, 0 }, // 5: bottom-right
	Vertex { 0.5f, 0.5f, -0.5f, 0, 1 }, // 6: top-right
	Vertex { -0.5f, 0.5f, -0.5f, 1, 1 }, // 7: top-left}
};

constexpr std::array<uint32_t, 36> example_cube_indices { // Front face
	{ 0, 1, 2, 2, 3, 0,
			// Back face
			4, 6, 5, 6, 4, 7,
			// Left face
			4, 0, 3, 3, 7, 4,
			// Right face
			1, 5, 6, 6, 2, 1,
			// Top face
			3, 2, 6, 6, 7, 3,
			// Bottom face
			4, 5, 1, 1, 0, 4 }
};

} //namespace

void Renderer::_bind_members() { ClassDB::bind_method(&Renderer::_render_scene, "_render_scene"); }

Renderer::Renderer() : _window(&Engine::get().get_main_window()) {
	// Need to assess what to do for multi window situation
	_window->register_notification(Notification::WINDOW_RESIZED, bind_method(&Renderer::_on_resize, this));
}

SDL_Window* Renderer::_extract_internal_window(Window& window) { return window._internal_window; }

TriangleMesh Renderer::get_example_cube() {
	static TriangleMesh example_cube { { example_cube_vertices.begin(), example_cube_vertices.end() },
		{ example_cube_indices.begin(), example_cube_indices.end() } };
	return example_cube;
}

INPLACE_REGISTER_BEGIN(Renderer);
ClassDB::register_abstract_class<Renderer>();
INPLACE_REGISTER_END(Renderer);

} //namespace feather