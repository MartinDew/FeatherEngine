#include "renderer.h"

#include "framework/functions.h"
#include "main/engine.h"
#include "main/window.h"

namespace feather {
Renderer::Renderer() {
	// Need to assess what to do for multi window situation
	Window& main_window = Engine::get().get_main_window();
	main_window.register_notification(Notification::WINDOW_RESIZED, bind_method(&Renderer::_on_resize, this));
}

} //namespace feather