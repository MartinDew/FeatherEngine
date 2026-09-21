#include "math_module.h"

#include "math/transform.h"

namespace feather {

MathEcsModule::MathEcsModule() = default;

MathEcsModule::MathEcsModule(World& world) {
	// Transform derives from IComponent and was registered from ClassDB already. Vector3/Matrix/Color are
	// DirectX::SimpleMath aliases and can never carry FSTRUCT, so they are declared by C++ type here instead.
	world.register_component_type<Vector3>("Vector3");
	world.register_component_type<Matrix>("Matrix");
	world.register_component_type<Color>("Color");
}

} //namespace feather