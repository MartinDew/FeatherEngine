#include "renderer.h"

#include "framework/functions.h"
#include "main/engine.h"
#include "main/window.h"

namespace feather {

Renderer::Renderer() {
	// Need to assess what to do for multi window situation
	_window = &Engine::get().get_main_window();
	_window->register_notification(Notification::WINDOW_RESIZED, bind_method(&Renderer::_on_resize, this));
}

SDL_Window* Renderer::_extract_internal_window(Window& window) { return window._internal_window; }

} //namespace feather