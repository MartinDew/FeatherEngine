#include "engine.h"
#include "launch_settings.h"

#include <modules/modules.gen.h>

int main(int argc, char* argv[]) {
	feather::LaunchSettings launch_settings{ argc, argv };

	feather::register_modules();

	feather::Engine engine;

	engine.run();

	feather::unregister_modules();
}