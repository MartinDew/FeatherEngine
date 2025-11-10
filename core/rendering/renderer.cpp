#include "renderer.h"

#include "framework/functions.h"
#include "main/engine.h"
#include "main/window.h"

namespace feather {

constexpr std::array<Vector3, 8> Renderer::example_cube = {
	Vector3(-1.0f, -1.0f, -1.0f),
	Vector3(1.0f, -1.0f, -1.0f),
	Vector3(1.0f, 1.0f, -1.0f),
	Vector3(-1.0f, 1.0f, -1.0f),
	Vector3(-1.0f, -1.0f, 1.0f),
	Vector3(1.0f, -1.0f, 1.0f),
	Vector3(1.0f, 1.0f, 1.0f),
	Vector3(-1.0f, 1.0f, 1.0f),
};

constexpr std::array<uint32_t, 36> Renderer::example_cube_indices = {
	0, 1, 2, 2, 3, 0, // Back face
	4, 5, 6, 6, 7, 4, // Front face
	0, 4, 7, 7, 3, 0, // Left face
	1, 5, 6, 6, 2, 1, // Right face
	3, 2, 6, 6, 7, 3, // Top face
	0, 1, 5, 5, 4, 0 // Bottom face
};

Renderer::Renderer() {
	// Need to assess what to do for multi window situation
	_window = &Engine::get().get_main_window();
	_window->register_notification(Notification::WINDOW_RESIZED, bind_method(&Renderer::_on_resize, this));
}

SDL_Window* Renderer::_extract_internal_window(Window& window) {
	return window._internal_window;
}

} //namespace feather