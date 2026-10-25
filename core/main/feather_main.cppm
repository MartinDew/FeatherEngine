export module feather.core:main.feather_main;

export namespace feather {

// The executable owns main() and the module list; core only drives the engine.
struct ModuleHooks {
	void (*register_modules)();
	void (*unregister_modules)();
};

int run(int argc, char* argv[], ModuleHooks hooks);

} //namespace feather
