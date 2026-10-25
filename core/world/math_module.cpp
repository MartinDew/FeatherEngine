module feather.core;

namespace feather {

MathWorldModule::MathWorldModule() = default;

MathWorldModule::MathWorldModule(World& world) {
	// Transform is registered up front as a Component (register_core_features.cpp); these are
	// reflected value types rather than components, so they are declared here by C++ type.
	world.component<Vector3>("Vector3");
	world.component<Matrix>("Matrix");
	world.component<Color>("Color");
}

} //namespace feather
