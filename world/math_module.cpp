module feather.world;

namespace feather {

MathWorldModule::MathWorldModule() = default;

MathWorldModule::MathWorldModule(World world) {
	// Math types are Variant built-ins, not reflected classes, so flecs alone describes them. Modules whose systems
	// use them import this module first.
	world.component<Transform>("Transform");
	world.component<Vector3>("Vector3");
	world.component<Matrix>("Matrix");
	world.component<Color>("Color");
}

} //namespace feather
