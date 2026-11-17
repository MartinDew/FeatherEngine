module feather.world;

namespace feather {

MathWorldModule::MathWorldModule() = default;

MathWorldModule::MathWorldModule(World world) {
	// Math sits below world, so its types can't be marked FSTRUCT(Component); they are declared here by C++ type.
	// Modules whose systems use them import this module first.
	world.component<Transform>("Transform");
	world.component<Vector3>("Vector3");
	world.component<Matrix>("Matrix");
	world.component<Color>("Color");
}

} //namespace feather
