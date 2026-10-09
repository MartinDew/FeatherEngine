module;
#include <framework/reflection_macros.h>
#include <SDL3/SDL_video.h>
module feather.core;

namespace feather {

Renderer::Renderer() : _window(&Engine::get().get_main_window()) {
}

SDL_Window* Renderer::_extract_internal_window(Window& window) {
	return window._internal_window;
}

} //namespace feather
