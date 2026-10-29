export module feather.core:world.ecs_defs;

import std;
import :framework.static_string;

export namespace feather {

class SystemIterator;

// An entity's identity. Opaque outside core/world/*.cpp: it can only be handed back to the Ecs that issued it.
class EntityId {
	std::uint64_t _value = 0;

public:
	constexpr EntityId() = default;
	explicit constexpr EntityId(std::uint64_t value) : _value(value) {}

	[[nodiscard]] constexpr bool is_null() const { return _value == 0; }
	explicit constexpr operator bool() const { return _value != 0; }

	constexpr bool operator==(const EntityId& other) const = default;

	[[nodiscard]] constexpr std::uint64_t raw() const { return _value; }
};

// The phases of the default pipeline, in the order it runs them.
enum class SystemPhase : std::uint8_t {
	OnLoad,
	PostLoad,
	PreUpdate,
	OnUpdate,
	OnValidate,
	PostUpdate,
	PreStore,
	OnStore,
};

// What a term does with its component. None is a pure filter: it must match but yields no field.
enum class TermAccess : std::uint8_t {
	In,
	InOut,
	Out,
	None,
};

// Where a term looks for its component. Up and Cascade fall back to ancestors (ChildOf), e.g. the owning scene.
enum class TraverseFlag : std::uint8_t {
	Self,
	Up,
	Cascade,
};

// Components are named, not typed, so the same description serves C++ types and ClassInfo-registered ones.
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

// Every system callback is reduced to this: `ctx` holds the user's callable, which only the trampoline knows the type of.
using SystemCallback = void (*)(SystemIterator& it, void* ctx);
using ContextDeleter = void (*)(void* ctx);

struct SystemDesc {
	QueryDesc query;

	EntityId phase;
	bool multi_threaded = false;
	// Null unless the system runs on a timer instead of every frame.
	EntityId tick_source;

	SystemCallback callback = nullptr;
	void* callback_ctx = nullptr;
	ContextDeleter callback_ctx_free = nullptr;

	// Runs once per invocation even when nothing matched, as a begin/commit pass needs.
	bool run_once = false;
};

} //namespace feather
