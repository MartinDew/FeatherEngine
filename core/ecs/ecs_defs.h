#pragma once

#include <framework/static_string.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace feather {

class SystemIterator;

// An entity's identity: a flecs entity id that nothing outside core/ecs/*.cpp can do anything with but pass back.
// Not Variant-marshalable, so a reflected signature names components by string and entities by Entity.
class EntityId {
	uint64_t _value = 0;

public:
	constexpr EntityId() = default;
	explicit constexpr EntityId(uint64_t value) : _value(value) {}

	[[nodiscard]] constexpr bool is_null() const { return _value == 0; }
	explicit constexpr operator bool() const { return _value != 0; }

	constexpr bool operator==(const EntityId& other) const = default;

	[[nodiscard]] constexpr uint64_t raw() const { return _value; }
};

// The phases the default pipeline runs, in this order -- the same set and order flecs' own default pipeline uses.
// World::phase turns one into the entity a system depends on; World::create_phase makes one that is not on this list.
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

// What a system does with a term's data. None is a filter: the term must match, but it yields no field.
enum class TermAccess : uint8_t {
	In,
	InOut,
	Out,
	None,
};

// Where a term looks for its component. Up walks to an ancestor rather than reading the entity itself.
enum class TraverseFlag : uint8_t {
	Self,
	Up,
	Cascade,
};

// One constraint in a query. Components are carried by name, which is what lets the same description serve a C++
// type and one a plugin described at runtime.
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

// The shape every system callback is reduced to: `ctx` holds the user's callable, and the trampoline that wrapped it
// is the only thing that still knows its type.
using SystemCallback = void (*)(SystemIterator& it, void* ctx);
using ContextDeleter = void (*)(void* ctx);

struct SystemDesc {
	QueryDesc query;

	// The phase entity the system runs in. A null phase leaves it out of every pipeline, so nothing schedules it.
	EntityId phase;

	// Added to the system entity, so a pipeline built around that tag matches it. Null means the default pipeline.
	EntityId pipeline_tag;

	bool multi_threaded = false;

	// Null unless the system is driven by a timer rather than by every frame.
	EntityId tick_source;

	SystemCallback callback = nullptr;
	void* callback_ctx = nullptr;
	ContextDeleter callback_ctx_free = nullptr;

	// True: runs once per invocation whether or not anything matched, which is what a begin/commit pass needs.
	bool run_once = false;
};

} //namespace feather
