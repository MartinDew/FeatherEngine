#pragma once

// Wraps a block of code in a static-initialised struct so it runs before main().
#define REGISTRATION_SCOPE_BEGIN                                                                                       \
	struct REGISTRATION_SCOPE {                                                                                        \
		REGISTRATION_SCOPE() {
#define REGISTRATION_SCOPE_END                                                                                         \
	}                                                                                                                  \
	}                                                                                                                  \
	REGISTRATION_SCOPE_instance;
