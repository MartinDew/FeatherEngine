#pragma once

#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <world/world.h>

namespace feather {
class World;

class EcsModule : public Reflected {
	FCLASS(EcsModule, Reflected);

protected:
	static void _bind_members() {};
	EcsModule() = default;

	static World* _get_world();
};

// Declare this within modules _bind_members funcs to automatically add them to the ECS
#define FBIND_MODULE()                                                                                                   \
	{ \
	void (*func)() = []() { _get_world()->import_module<Type>(); };\
	ClassDB::bind_static_method(func, "_import_module");\
	} static_assert(true)

} //namespace feather
