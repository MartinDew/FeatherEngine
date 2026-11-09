module;

#include "framework/reflection_macros.h"

#ifndef FEATHER_REFLECTION_PARSER
#include "simulation.gen.h"
#endif

export module feather.world:simulation;

import std;
import feather.framework;
import feather.reflection;

export namespace feather {

// class that runs the engine main loop.
// It manages the main logic for execution of entities, components and such.
class FEATHER_WORLD_API Simulation : public Reflected {
	FCLASS();

protected:
	Simulation() = default;

public:
	[[method]] inline virtual void init() {};
	inline virtual void pre_update(double delta) {};
	[[method]] inline virtual void fixed_update(double delta) {};
	[[method]] inline virtual void update(double delta) {};

	~Simulation() override = default;
};

} //namespace feather
