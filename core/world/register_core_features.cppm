module;

export module feather.core:world.register_core_features;

import :world.ecs;

export namespace feather {

// Registers every reflected Component up front, so they exist before any EcsObject's systems name them.
void register_core_components(Ecs& ecs);

} // namespace feather
