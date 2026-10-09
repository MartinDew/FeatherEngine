#pragma once

#include "ecs_defs.h"
#include "system_iterator.h"
#include "world.h"

#include <framework/export_defs.h>

#include <string>
#include <utility>

namespace feather {

// Describes one system and registers it. The pack becomes the data terms in order -- `T` required and written,
// `const T` read-only, `T*` optional and null where absent; with() adds a filter term that yields no data.
template <class... TComps>
class SystemBuilder {
	World* _world = nullptr;
	SystemDesc _desc;

public:
	SystemBuilder(World& world, const char* name) : _world(&world) {
		_desc.query.name = name ? name : "";
		_desc.query.terms = ecs_detail::make_terms<TComps...>();
		// The phase a system gets when it never names one, matching where flecs puts an unqualified system.
		_desc.phase = world.phase(SystemPhase::OnUpdate);
	}

	// One of the phases the default pipeline runs.
	SystemBuilder& phase(SystemPhase phase) {
		_desc.phase = _world->phase(phase);
		return *this;
	}

	// A phase of your own, from World::create_phase.
	SystemBuilder& phase(EntityId phase) {
		_desc.phase = phase;
		return *this;
	}

	// Puts the system in the set a pipeline built around `tag` runs; see World::create_pipeline.
	SystemBuilder& pipeline_tag(EntityId tag) {
		_desc.pipeline_tag = tag;
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

	// Runs `fn` once per invocation, handed the whole batch. Unlike each(), this still fires when nothing matched,
	// which is what a system with no terms is for.
	EntityId run(void (*fn)(SystemIterator&)) {
		using RunFn = void (*)(SystemIterator&);

		_desc.callback_ctx = new RunFn(fn);
		_desc.callback = [](SystemIterator& it, void* ctx) { (*static_cast<RunFn*>(ctx))(it); };
		_desc.callback_ctx_free = [](void* ctx) { delete static_cast<RunFn*>(ctx); };
		_desc.run_once = true;
		return _world->_register_system(std::move(_desc));
	}
};

template <class... TComps>
SystemBuilder<TComps...> World::system(const char* name) {
	return SystemBuilder<TComps...>(*this, name);
}

} //namespace feather
