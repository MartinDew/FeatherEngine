#pragma once

#include "ecs_defs.h"
#include "entity.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace feather {

// One batch of entities handed to a running system or query.
//
// This is the firewall. Underneath it is a flecs `ecs_iter_t`, but every method here is non-template and defined in
// system_iterator.cpp, so a header that iterates -- including the templates in system_builder.h -- never needs a flecs
// symbol. `field<T>()` is the only template, and it does nothing but cast what the non-template `_field_ptr` returned.
class FEATHER_API SystemIterator {
	// ecs_iter_t*
	void* _impl = nullptr;
	World* _world = nullptr;

public:
	SystemIterator() = default;
	SystemIterator(World& world, void* impl) : _impl(impl), _world(&world) {}

	[[nodiscard]] size_t count() const;
	[[nodiscard]] float delta_time() const;
	[[nodiscard]] Entity entity_at(size_t row) const;
	[[nodiscard]] World& get_world() const { return *_world; }

	// The storage for one term of this iteration, or nullptr when the term is optional and did not match.
	// `size` is checked against what the component was registered with, which is why this can stay non-template.
	[[nodiscard]] void* _field_ptr(int32_t term, size_t size) const;

	template <typename T>
	[[nodiscard]] T* field(int32_t term) const {
		return static_cast<T*>(_field_ptr(term, sizeof(std::remove_const_t<T>)));
	}
};

namespace ecs_detail {

// How one element of a system's component pack is spelled, and how it is handed to the callback.
// `T` is a required term yielding `T&`; `T*` is an optional term yielding `T*`, which is null on the rows where the
// component is absent. Constness of the term is what tells the ECS whether the system reads or writes it.
template <class T>
struct TermTraits {
	using Component = std::remove_const_t<T>;
	static constexpr bool is_optional = false;
	static constexpr TermAccess access = std::is_const_v<T> ? TermAccess::In : TermAccess::InOut;

	static T& row(void* field, size_t i) { return static_cast<T*>(field)[i]; }
};

template <class T>
struct TermTraits<T*> {
	using Component = std::remove_const_t<T>;
	static constexpr bool is_optional = true;
	static constexpr TermAccess access = std::is_const_v<T> ? TermAccess::In : TermAccess::InOut;

	static T* row(void* field, size_t i) { return field ? static_cast<T*>(field) + i : nullptr; }
};

template <class T>
struct TermTraits<T&> : TermTraits<T> {};

// The pack turned into a term list, naming each component by its reflected class name. This is the whole reason the
// builder templates need no flecs: a name and a size is all the description a term carries.
template <class... TComps>
std::vector<Term> make_terms() {
	std::vector<Term> terms;
	terms.reserve(sizeof...(TComps));
	(terms.push_back(Term {
			 TermTraits<TComps>::Component::get_class_static(),
			 TermTraits<TComps>::access,
			 TraverseFlag::Self,
			 TermTraits<TComps>::is_optional,
	 }),
	 ...);
	return terms;
}

template <class... TComps, class F, size_t... Is>
void invoke_each(SystemIterator& it, F& fn, std::index_sequence<Is...>) {
	// Resolved once per batch rather than per row: every row of this batch reads the same table column.
	void* fields[] = { it._field_ptr(static_cast<int32_t>(Is), sizeof(typename TermTraits<TComps>::Component))... };

	const size_t rows = it.count();
	for (size_t row = 0; row < rows; ++row) {
		fn(it.entity_at(row), TermTraits<TComps>::row(fields[Is], row)...);
	}
}

// A system with no component terms still iterates entities; the array above would be zero-sized, which C++ forbids.
template <class F>
void invoke_each(SystemIterator& it, F& fn, std::index_sequence<>) {
	const size_t rows = it.count();
	for (size_t row = 0; row < rows; ++row) {
		fn(it.entity_at(row));
	}
}

// Wraps a callable into the type-erased SystemCallback pair the ECS stores. The callable is copied to the heap because
// it has to outlive the builder; `free` gives the ECS a way to destroy it without knowing its type.
template <class... TComps, class F>
void bind_each(F&& fn, SystemCallback& out_callback, void*& out_ctx, ContextDeleter& out_free) {
	using Fn = std::decay_t<F>;

	out_ctx = new Fn(std::forward<F>(fn));
	out_callback = [](SystemIterator& it, void* ctx) {
		invoke_each<TComps...>(it, *static_cast<Fn*>(ctx), std::index_sequence_for<TComps...> {});
	};
	out_free = [](void* ctx) { delete static_cast<Fn*>(ctx); };
}

} //namespace ecs_detail

} //namespace feather
