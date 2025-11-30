#include "register_module.h"

#include "vex_renderer.h"

#include <core/rendering/renderer.h>

namespace feather {
void register_vex_renderer() {
	get_renderer_setting().register_resolver("vex"_ss, [] { return std::make_unique<VexRenderer>(); });
}

void unregister_vex_renderer() {}
} //namespace feather