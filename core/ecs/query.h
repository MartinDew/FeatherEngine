#pragma once

#include "ecs_defs.h"
#include "system_iterator.h"
#include "world.h"

#include <framework/export_defs.h>

#include <string>
#include <utility>

namespace feather {

// A reusable set of entities matching a list of terms -- a system's description without the schedule. Move-only:
// it owns the underlying flecs query, which its destructor hands back to World.
template <class... TComps>
class Query {
	World* _world = nullptr;
	// ecs_query_t*, owned.
	void* _impl = nullptr;

public:
	Query() = default;
	Query(World& world, QueryDesc&& desc) : _world(&world), _impl(world._create_query(std::move(desc))) {}

	~Query() {
		if (_world && _impl) {
			_world->_destroy_query(_impl);
		}
	}

	Query(const Query&) = delete;
	Query& operator=(const Query&) = delete;

	Query(Query&& other) noexcept : _world(other._world), _impl(other._impl) {
		other._world = nullptr;
		other._impl = nullptr;
	}

	Query& operator=(Query&& other) noexcept {
		if (this != &other) {
			if (_world && _impl) {
				_world->_destroy_query(_impl);
			}
			_world = other._world;
			_impl = other._impl;
			other._world = nullptr;
			other._impl = nullptr;
		}
		return *this;
	}

	[[nodiscard]] bool is_valid() const { return _impl != nullptr; }
	explicit operator bool() const { return is_valid(); }

	// Runs `fn` for every entity matched, with the pack's components as arguments after the entity. Synchronous, so
	// the callable need not outlive the call -- unlike a system's, it is not copied to the heap.
	template <class F>
	void each(F&& fn) const {
		if (!_impl) {
			return;
		}
		auto callable = std::forward<F>(fn);
		_world->_query_each(
				_impl,
				[](SystemIterator& it, void* ctx) {
					ecs_detail::invoke_each<TComps...>(
							it, *static_cast<decltype(callable)*>(ctx), std::index_sequence_for<TComps...> {}
					);
				},
				&callable
		);
	}
};

// Describes a query the same way SystemBuilder describes a system, and builds it.
template <class... TComps>
class QueryBuilder {
	World* _world = nullptr;
	QueryDesc _desc;

public:
	QueryBuilder(World& world, const char* name = "") : _world(&world) {
		_desc.name = name ? name : "";
		_desc.terms = ecs_detail::make_terms<TComps...>();
	}

	QueryBuilder& with(StaticString component, TraverseFlag traverse = TraverseFlag::Self) {
		_desc.terms.push_back(Term { component, TermAccess::None, traverse, false });
		return *this;
	}

	template <class T>
	QueryBuilder& with(TraverseFlag traverse = TraverseFlag::Self) {
		return with(T::get_class_static(), traverse);
	}

	[[nodiscard]] Query<TComps...> build() { return Query<TComps...>(*_world, std::move(_desc)); }
};

} //namespace feather
