#include "renderer.h"

#include "framework/functions.h"
#include "framework/reflection_macros.h"
#include "main/class_db.h"
#include "main/engine.h"
#include "main/window.h"
#include "rendering/mesh_data.h"

namespace feather {

void Renderer::_bind_members() { ClassDB::bind_method(&Type::_render_scene, "_render_scene"); }

Renderer::Renderer() : _window(&Engine::get().get_main_window()) {}

SDL_Window* Renderer::_extract_internal_window(Window& window) { return window._internal_window; }

INPLACE_REGISTER_BEGIN(Renderer);
ClassDB::register_abstract_class<Renderer>();
INPLACE_REGISTER_END(Renderer);

} //namespace feather