module;

#include <flecs.h>
#include <flecs/addons/cpp/entity.hpp>

export module feather.core:world.ecs_defs;

export namespace feather {
using Entity = flecs::entity;
using World = flecs::world;
namespace Ecs = flecs;
using EcsTimer = flecs::timer;
} //namespace feather
