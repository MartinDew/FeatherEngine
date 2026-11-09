module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "scene.gen.h"
#endif

export module feather.world:components.scene;

import std.compat;
import feather.framework;
import :ecs_defs;
import feather.reflection;

export namespace feather {

struct FEATHER_WORLD_API Scene {
	FSTRUCT(Component);

	[[get, set]] int64_t _scene_id = -1;

	Scene() = default;
	Scene(StaticString name);
	Scene(const Scene&) = default;
	Scene& operator=(const Scene&) = default;
	Scene(Scene&&) = default;
	Scene& operator=(Scene&&) = default;

	bool operator==(const Scene& other) const;
};

// Relationship Tag
struct FEATHER_WORLD_API ActiveScene {
	FSTRUCT(Component);
};

struct FEATHER_WORLD_API InScene {
	FSTRUCT(Component);
};

} //namespace feather
