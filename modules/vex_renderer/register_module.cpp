module;

#include "register_vex_renderer_types.gen.h"

module feather.vex_renderer;

namespace feather {

void register_vex_renderer() {
	register_vex_renderer_types();
}

void unregister_vex_renderer() {
}

} //namespace feather