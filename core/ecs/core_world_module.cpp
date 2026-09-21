#include "core_world_module.h"

#include "components/scene.h"

namespace feather {

// Scene/ActiveScene/InScene reach the ECS through World::register_classdb_components, so the module declares nothing.
void CoreEcsModule::on_import(World& world) {
}

} //namespace feather
