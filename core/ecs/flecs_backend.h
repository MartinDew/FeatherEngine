#pragma once

// The one place flecs is named. Included only from core/ecs/*.cpp -- never from a header -- which is what keeps the
// rest of the engine, and every project DLL, from seeing it. If you find yourself wanting this in a header, the thing
// you need is probably missing from World instead.

#include "ecs_defs.h"

#include <flecs.h>

namespace feather {

inline ecs_entity_t to_flecs_phase(SystemPhase phase) {
	switch (phase) {
		case SystemPhase::OnLoad:
			return EcsOnLoad;
		case SystemPhase::PostLoad:
			return EcsPostLoad;
		case SystemPhase::PreUpdate:
			return EcsPreUpdate;
		case SystemPhase::OnUpdate:
			return EcsOnUpdate;
		case SystemPhase::OnValidate:
			return EcsOnValidate;
		case SystemPhase::PostUpdate:
			return EcsPostUpdate;
		case SystemPhase::PreStore:
			return EcsPreStore;
		case SystemPhase::OnStore:
			return EcsOnStore;
	}
	return EcsOnUpdate;
}

inline int16_t to_flecs_inout(TermAccess access) {
	switch (access) {
		case TermAccess::In:
			return EcsIn;
		case TermAccess::InOut:
			return EcsInOut;
		case TermAccess::Out:
			return EcsOut;
		case TermAccess::None:
			return EcsInOutNone;
	}
	return EcsInOutDefault;
}

// Which relationship a term walks when it does not find its component on the entity itself. ChildOf rather than
// flecs' own default of IsA: Up here means "on an ancestor", such as a scene entity.
inline ecs_entity_t to_flecs_traversal_relationship(TraverseFlag traverse) {
	return traverse == TraverseFlag::Self ? 0 : EcsChildOf;
}

// Traversal is spelled as flags on the term's source, not as a separate field. Up and Cascade both imply looking at
// the entity itself first, which is what `.up()` does in the C++ builder.
inline ecs_id_t to_flecs_traverse_flags(TraverseFlag traverse) {
	switch (traverse) {
		case TraverseFlag::Self:
			return EcsSelf;
		case TraverseFlag::Up:
			return EcsSelf | EcsUp;
		case TraverseFlag::Cascade:
			return EcsSelf | EcsCascade;
	}
	return EcsSelf;
}

} //namespace feather
