module;

#include <flecs.h>
#include <flecs/addons/cpp/entity.hpp>

export module feather.world:ecs_defs;

export namespace feather {
using Entity = flecs::entity;
using EcsTimer = flecs::timer;
} //namespace feather
