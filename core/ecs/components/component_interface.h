#pragma once

#include "component_interface.gen.h"

namespace feather {

// Parent struct to all component types. Inheriting this for an exposed class means that it will be used as a component for the world
struct IComponent {
	FSTRUCT();
};

} //namespace feather

