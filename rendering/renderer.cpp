module;
#include <SDL3/SDL_video.h>
#include <framework/reflection_macros.h>
module feather.rendering;

namespace feather {

Renderer::Renderer() : _window(&EngineContext::get_main_window()) {
}

SDL_Window* Renderer::_extract_internal_window(Window& window) {
	return window.get_internal_window();
}

} //namespace feather
