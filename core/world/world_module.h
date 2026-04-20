#pragma once

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

namespace feather {

class WorldModule : public Reflected {
	FCLASS(WorldModule, Reflected);

protected:
	static void _bind_methods() {};
};

} //namespace feather