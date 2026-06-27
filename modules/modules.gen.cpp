// THIS FILE IS AUTO-GENERATED — DO NOT EDIT BY HAND.
// Re-run generate_modules.py to refresh.

#include "modules.gen.h"
#include <framework/assert.h>

#if vex_renderer_ENABLED
#include <modules/vex_renderer/register_module.h>
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
