module feather.modules;

import feather.main;

#if FEATHER_VEX_RENDERER_ENABLED
import feather.vex_renderer;
#endif

namespace feather {

static bool registered = false;

void register_modules() {
	fassert(!registered, "modules already registered but register_modules was called!");
#if FEATHER_VEX_RENDERER_ENABLED
	register_vex_renderer();
#endif

	registered = true;
}

void unregister_modules() {
	fassert(registered, "modules not registered but unregister was called!");
#if FEATHER_VEX_RENDERER_ENABLED
	unregister_vex_renderer();
#endif
	registered = false;
}

} //namespace feather