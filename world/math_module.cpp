module feather.world;

namespace feather {

MathWorldModule::MathWorldModule() = default;

MathWorldModule::MathWorldModule(World world) {
	// Math types are Variant built-ins, not ClassDB classes. Modules whose systems use them import this module first.
	world.register_builtin_component<Transform>();
	world.register_builtin_component<Vector3>();
	world.register_builtin_component<Matrix>();
	world.register_builtin_component<Color>();
}

} //namespace feather
