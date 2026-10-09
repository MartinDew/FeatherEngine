#include "math_module.h"

#include "math/transform.h"

namespace feather {

void MathEcsModule::on_import(World& world) {
	// Transform derives from IComponent and arrives from ClassDB. Vector3/Matrix/Color are DirectX::SimpleMath aliases
	// that can never carry FSTRUCT, so they are declared by C++ type here and named explicitly.
	world.register_component_type<Vector3>("Vector3");
	world.register_component_type<Matrix>("Matrix");
	world.register_component_type<Color>("Color");
}

} //namespace feather
