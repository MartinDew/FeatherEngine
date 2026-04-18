#pragma once
#include "framework/reflected.h"
#include "framework/reflection_macros.h"

namespace feather {

// class that runs the engine main loop.
// It manages the main logic for execution of entities, components and such.
class Simulation : public Reflected {
	FABSTRACT(Simulation, Reflected);

protected:
	static void _bind_members();

	Simulation() = default;

public:
	virtual void init() {};
	virtual void pre_update(double delta) {};
	virtual void fixed_update(double delta) {};
	virtual void update(double delta) {};

	~Simulation() override = default;
};

} //namespace feather