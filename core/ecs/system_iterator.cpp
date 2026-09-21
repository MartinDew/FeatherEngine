#include "system_iterator.h"

#include "flecs_backend.h"

namespace feather {

size_t SystemIterator::count() const {
	return _impl ? static_cast<size_t>(static_cast<ecs_iter_t*>(_impl)->count) : 0;
}

float SystemIterator::delta_time() const {
	return _impl ? static_cast<float>(static_cast<ecs_iter_t*>(_impl)->delta_time) : 0.0f;
}

Entity SystemIterator::entity_at(size_t row) const {
	if (!_impl || !_world) {
		return {};
	}
	const auto* it = static_cast<ecs_iter_t*>(_impl);
	if (row >= static_cast<size_t>(it->count)) {
		return {};
	}
	return { *_world, EntityId(it->entities[row]) };
}

void* SystemIterator::_field_ptr(int32_t term, size_t size) const {
	if (!_impl) {
		return nullptr;
	}
	auto* it = static_cast<ecs_iter_t*>(_impl);
	if (term < 0 || term >= it->field_count) {
		return nullptr;
	}
	// Null for an optional term that did not match on this batch, which is exactly what the caller hands on to the
	// system as a null component pointer.
	return ecs_field_w_size(it, size, static_cast<int8_t>(term));
}

} //namespace feather
