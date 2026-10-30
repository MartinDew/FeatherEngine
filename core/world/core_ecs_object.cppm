module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "core_ecs_object.gen.h"
#endif

export module feather.core:world.core_ecs_object;

import std;
import :framework.static_string;
import :world.ecs;
import :world.ecs_object;
import :main.class_db;

export namespace feather {

class FEATHER_API CoreEcsObject : public EcsObject {
	FCLASS(EcsObject);

public:
	CoreEcsObject() = default;

	void on_import(Ecs& ecs) override;
};

} //namespace feather
