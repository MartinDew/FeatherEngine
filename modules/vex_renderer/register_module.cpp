#include "register_module.h"

#include "vex_renderer.h"

#include <core/rendering/renderer.h>
#include <core/rendering/rendering_server.h>

#include <core/main/class_db.h>

namespace feather {

void register_vex_renderer() { ClassDB::register_class<VexRenderer>(); }

void unregister_vex_renderer() {}

} //namespace feather