#pragma once

#include <flecs.h>
#include <flecs/addons/cpp/entity.hpp>

namespace feather {
// flecs' own entity handle. Feather's frontend is world/entity.h's Entity; this
// is what a system callback is handed, and what the query builders speak.
using EcsEntity = flecs::entity;
namespace Ecs = flecs;
using EcsTimer = flecs::timer;
} //namespace feather
