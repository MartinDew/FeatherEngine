#include "engine.h"
#include "launch_settings.h"

#include <modules/modules.gen.h>

int main(int argc, char* argv[]) {
	feather::register_modules();
	feather::LaunchSettings::get().init(argc, argv);

	feather::Engine engine;

	engine.run();

	feather::unregister_modules();
}