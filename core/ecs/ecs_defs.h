#pragma once

#include <framework/static_string.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace feather {

class SystemIterator;

// An entity's identity, and the only thing that crosses the ECS boundary in place of a flecs handle.
//
// Opaque on purpose: the underlying value is a flecs entity id, but nothing outside core/ecs/*.cpp may do anything
// with it except pass it back. Deliberately not Variant-marshalable -- it is 64 bits and a Variant's integer is not --
// so every reflected signature names components by string and entities by Entity instead.
class EntityId {
	uint64_t _value = 0;

public:
	constexpr EntityId() = default;
	explicit constexpr EntityId(uint64_t value) : _value(value) {}

	[[nodiscard]] constexpr bool is_null() const { return _value == 0; }
	explicit constexpr operator bool() const { return _value != 0; }

	constexpr bool operator==(const EntityId& other) const = default;

	// The raw flecs id. Only core/ecs/*.cpp can do anything with it.
	[[nodiscard]] constexpr uint64_t raw() const { return _value; }
};

// When a system runs within a frame. Mirrors the flecs pipeline phases, so that a module names a phase without
// naming flecs -- the mapping lives in core/ecs/flecs_backend.h.
enum class SystemPhase : uint8_t {
	OnLoad,
	PostLoad,
	PreUpdate,
	OnUpdate,
	OnValidate,
	PostUpdate,
	PreStore,
	OnStore,
};

// What a system does with a term's data. `None` is a filter: the term must match, but it yields no field, which is
// what a tag like ActiveScene is for.
enum class TermAccess : uint8_t {
	In,
	InOut,
	Out,
	None,
};

// Where a term looks for its component. `Up` walks to a parent -- how a scene-scoped system finds the ActiveScene tag
// on an ancestor rather than on the entity itself.
enum class TraverseFlag : uint8_t {
	Self,
	Up,
	Cascade,
};

// One constraint in a query. Components are carried by name, not by type: that is what lets the same description
// serve a C++ type and one a plugin described at runtime.
struct Term {
	StaticString component = ""_ss;
	TermAccess access = TermAccess::InOut;
	TraverseFlag traverse = TraverseFlag::Self;
	bool optional = false;
};

struct QueryDesc {
	std::string name;
	std::vector<Term> terms;
};

// The type-erased shape every system callback is reduced to. The template that built it is long gone by the time
// this runs: `ctx` holds the user's callable, and the trampoline in system_builder.h knows how to unpack it.
using SystemCallback = void (*)(SystemIterator& it, void* ctx);
using ContextDeleter = void (*)(void* ctx);

struct SystemDesc {
	QueryDesc query;
	SystemPhase phase = SystemPhase::OnUpdate;
	bool multi_threaded = false;
	// Zero unless the system is driven by a timer rather than by every frame.
	EntityId tick_source;

	SystemCallback callback = nullptr;
	void* callback_ctx = nullptr;
	ContextDeleter callback_ctx_free = nullptr;

	// False: `callback` runs once per matching batch of entities, which is what each() wants.
	// True: it runs once per system invocation whether or not anything matched, which is what a system with no terms
	// needs -- a frame's begin/commit pass has nothing to match but must still fire.
	bool run_once = false;
};

} //namespace feather
