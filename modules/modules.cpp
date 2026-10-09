module feather.modules;

import feather.core;

#if vex_renderer_ENABLED
import feather.vex_renderer;
#endif

namespace feather {

static bool registered = false;

void register_modules() {
	fassert(!registered, "modules already registered but register_modules was called!");
#if vex_renderer_ENABLED
	register_vex_renderer();
#endif

	registered = true;
}

void unregister_modules() {
	fassert(registered, "modules not registered but unregister was called!");
#if vex_renderer_ENABLED
	unregister_vex_renderer();
#endif
	registered = false;
}

} //namespace feather