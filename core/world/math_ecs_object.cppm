module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "math_ecs_object.gen.h"
#endif

export module feather.core:world.math_ecs_object;

import std;
import :framework.static_string;
import :world.ecs;
import :world.ecs_object;
import :main.class_db;

export namespace feather {

class FEATHER_API MathEcsObject final : public EcsObject {
	FCLASS(EcsObject);

public:
	MathEcsObject() = default;

	void on_import(Ecs& ecs) override;
};

} //namespace feather
