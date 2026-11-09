export module feather.main:feather_main;

export namespace feather {

// The executable owns main() and the module list; core only drives the engine.
struct FEATHER_MAIN_API ModuleHooks {
	void (*register_modules)();
	void (*unregister_modules)();
};

FEATHER_MAIN_API int run(int argc, char* argv[], ModuleHooks hooks);

} //namespace feather
