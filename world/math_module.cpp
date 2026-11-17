module feather.world;

namespace feather {

MathWorldModule::MathWorldModule() = default;

MathWorldModule::MathWorldModule(World world) {
	// Math sits below world, so its types can't be marked FSTRUCT(Component) and are registered here instead.
	// Modules whose systems use them import this module first. Transform is not reflected, so flecs alone describes it.
	world.component<Transform>("Transform");
	world.register_component<Vector3>();
	world.register_component<Matrix>();
	world.register_component<Color>();
}

} //namespace feather
