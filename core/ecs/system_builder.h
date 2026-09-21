#pragma once

#include "ecs_defs.h"
#include "system_iterator.h"
#include "world.h"

#include <framework/export_defs.h>

#include <string>
#include <utility>

namespace feather {

// Describes one system and registers it with the world.
//
// The component pack becomes the system's data terms, in order: a plain `T` is a required term the system writes, a
// `const T` one it only reads, and a `T*` an optional term that is null on rows where the component is absent. Terms
// added afterwards with with() are filters -- they constrain what matches but yield no data.
//
// Nothing here names a flecs type. The pack is reduced to component names and sizes (system_iterator.h), the callback
// to a type-erased trampoline, and the whole description handed to World::_register_system, which is where flecs is.
//
// Construct one through EcsModule::system(): a system belongs to the module that declares it.
template <class... TComps>
class SystemBuilder {
	World* _world = nullptr;
	SystemDesc _desc;

public:
	SystemBuilder(World& world, const char* name) : _world(&world) {
		_desc.query.name = name ? name : "";
		_desc.query.terms = ecs_detail::make_terms<TComps...>();
	}

	SystemBuilder& phase(SystemPhase phase) {
		_desc.phase = phase;
		return *this;
	}

	SystemBuilder& multi_threaded(bool enabled = true) {
		_desc.multi_threaded = enabled;
		return *this;
	}

	// Drives the system from a timer rather than from every frame; see World::create_timer.
	SystemBuilder& tick_source(EntityId timer) {
		_desc.tick_source = timer;
		return *this;
	}

	// A filter term: the component must be present, but the system is handed no data for it. `traverse` decides where
	// it is looked for -- Up finds it on an ancestor, which is how a system is scoped to the active scene.
	SystemBuilder& with(StaticString component, TraverseFlag traverse = TraverseFlag::Self) {
		_desc.query.terms.push_back(Term { component, TermAccess::None, traverse, false });
		return *this;
	}

	template <class T>
	SystemBuilder& with(TraverseFlag traverse = TraverseFlag::Self) {
		return with(T::get_class_static(), traverse);
	}

	// Runs `fn` once for every entity matched, with the pack's components as arguments after the entity.
	template <class F>
	EntityId each(F&& fn) {
		ecs_detail::bind_each<TComps...>(
				std::forward<F>(fn), _desc.callback, _desc.callback_ctx, _desc.callback_ctx_free
		);
		_desc.run_once = false;
		return _world->_register_system(std::move(_desc));
	}

	// Runs `fn` once per system invocation, handed the whole batch. Unlike each(), this still fires when nothing
	// matched, which is what a system with no terms is for.
	EntityId run(void (*fn)(SystemIterator&)) {
		using RunFn = void (*)(SystemIterator&);

		_desc.callback_ctx = new RunFn(fn);
		_desc.callback = [](SystemIterator& it, void* ctx) { (*static_cast<RunFn*>(ctx))(it); };
		_desc.callback_ctx_free = [](void* ctx) { delete static_cast<RunFn*>(ctx); };
		_desc.run_once = true;
		return _world->_register_system(std::move(_desc));
	}
};

} //namespace feather
