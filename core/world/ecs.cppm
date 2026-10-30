module;

#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs.gen.h"
#endif

export module feather.core:world.ecs;

import std;
import :framework.static_string;
import :framework.class_info;
import :framework.reflected;
import :framework.variant;
import :world.ecs_defs;
import :main.class_db;

export namespace feather {

class Ecs;
class EcsObject;
class ComponentHandle;

template <class... TComps>
class SystemBuilder;

template <class... TComps>
class QueryBuilder;

// A type ClassDB names. That name is the component's identity in the ECS, so it is the same in the exe and every DLL.
template <typename T>
concept ClassDbNamed = requires {
	{
		T::get_class_static()
	} -> std::convertible_to<StaticString>;
};

// A handle: an Ecs and an id, no storage of its own.
class FEATHER_API Entity {
	Ecs* _ecs = nullptr;
	EntityId _id;

public:
	Entity() = default;
	Entity(Ecs& ecs, EntityId id) : _ecs(&ecs), _id(id) {}

	[[nodiscard]] EntityId id() const { return _id; }
	[[nodiscard]] Ecs* get_ecs() const { return _ecs; }

	bool operator==(const Entity& other) const { return _ecs == other._ecs && _id == other._id; }

	[[nodiscard]] bool is_valid() const;
	[[nodiscard]] std::string get_name() const;
	void set_name(const std::string& name);
	void destroy();

	Entity& child_of(const Entity& parent);
	Entity& is_a(const Entity& prefab);
	[[nodiscard]] Entity get_parent() const;
	[[nodiscard]] bool is_descendant_of(const Entity& ancestor) const;
	[[nodiscard]] bool is_instance_of(const Entity& prefab) const;

	// By name, for components with no C++ type at hand.
	Entity& add_component(StaticString component);
	[[nodiscard]] bool has_component(StaticString component) const;
	Entity& remove_component(StaticString component);
	[[nodiscard]] ComponentHandle component(StaticString component) const;

	template <ClassDbNamed T>
	Entity& add();

	template <ClassDbNamed T>
	Entity& set(T value);

	template <ClassDbNamed T, class... TArgs>
	Entity& emplace(TArgs&&... args);

	template <ClassDbNamed T>
	[[nodiscard]] const T* get() const;

	template <ClassDbNamed T>
	[[nodiscard]] T* get_mutable();

	template <ClassDbNamed T>
	[[nodiscard]] bool has() const;

	template <ClassDbNamed T>
	Entity& remove();
};

// One batch handed to a running system or query. Non-template members only, so iterating never needs flecs.
class FEATHER_API SystemIterator {
	void* _impl = nullptr; // ecs_iter_t*
	Ecs* _ecs = nullptr;

public:
	SystemIterator() = default;
	SystemIterator(Ecs& ecs, void* impl) : _impl(impl), _ecs(&ecs) {}

	[[nodiscard]] std::size_t count() const;
	[[nodiscard]] float delta_time() const;
	[[nodiscard]] Entity entity_at(std::size_t row) const;
	[[nodiscard]] Ecs& get_ecs() const { return *_ecs; }

	// Null when the term is optional and absent from this batch.
	[[nodiscard]] void* _field_ptr(std::int32_t term, std::size_t size) const;

	template <class T>
	[[nodiscard]] T* field(std::int32_t term) const {
		return static_cast<T*>(_field_ptr(term, sizeof(std::remove_const_t<T>)));
	}
};

// An ECS world. flecs lives behind _impl; every component is identified by its class name.
class FEATHER_API Ecs : public Reflected {
	FCLASS();

	friend class Entity;
	friend class SystemIterator;
	friend class ComponentHandle;
	template <class... TComps>
	friend class SystemBuilder;
	template <class... TComps>
	friend class Query;

	struct Impl;
	std::unique_ptr<Impl> _impl;

	std::unordered_map<StaticString, EntityId> _components;
	// The reflection each component was registered from; absent for components without a ClassInfo.
	std::unordered_map<uint64_t, const ClassInfo*> _component_infos;

	std::unordered_map<StaticString, EntityId> _objects;
	std::vector<std::unique_ptr<EcsObject>> _object_instances;

	// flecs keeps the names it is handed, and StaticString is only a view.
	static StaticString _intern(std::string_view name);

	EntityId _register_component(
			StaticString name,
			std::size_t size,
			std::size_t alignment,
			const ValueTypeHooks& hooks,
			const ClassInfo* info
	);

	void _link_class_info(EntityId component, const ClassInfo& info);

	// Copies or moves one instance into the entity's storage, adding the component first if needed.
	bool _set_component(EntityId entity, EntityId component, void* value, std::size_t size, bool move);
	[[nodiscard]] const void* _component_data(EntityId entity, EntityId component) const;
	[[nodiscard]] void* _mutable_component_data(EntityId entity, EntityId component);

	EntityId _register_system(SystemDesc&& desc);

	// Owned ecs_query_t*, handed back through _destroy_query.
	[[nodiscard]] void* _create_query(QueryDesc&& desc);
	void _destroy_query(void* query) const;
	void _query_each(void* query, SystemCallback callback, void* ctx);

public:
	Ecs();
	~Ecs() override;

	Ecs(const Ecs&) = delete;
	Ecs& operator=(const Ecs&) = delete;

	// ---- Simulation ----

	// A delta of 0 lets the ECS measure the frame itself.
	bool progress(double delta = 0.0);
	[[nodiscard]] EntityId create_timer(double interval);
	[[nodiscard]] float delta_time() const;
	void enable_rest_api();
	[[nodiscard]] EntityId phase(SystemPhase phase) const;

	// A pipeline running only the systems assigned to it (SystemBuilder::pipeline), in phase order. Assigned
	// systems leave the default pipeline.
	EntityId create_pipeline(const std::string& name);
	[[nodiscard]] EntityId get_default_pipeline() const;
	// The pipeline progress() runs.
	[[nodiscard]] EntityId get_pipeline() const;
	void set_pipeline(EntityId pipeline);
	// Runs `pipeline` once without making it current. A delta of 0 reuses the last frame's.
	void run_pipeline(EntityId pipeline, double delta = 0.0);

	// A phase of your own that the default pipeline runs right after `runs_after`. Phases chain, so `runs_after` may
	// itself be a custom phase; a null one leaves the phase unordered.
	EntityId create_phase(const std::string& name, EntityId runs_after);
	EntityId create_phase(const std::string& name, SystemPhase runs_after) {
		return create_phase(name, phase(runs_after));
	}

	// ---- Entities ----

	[[nodiscard]] Entity entity(const std::string& name = "");
	[[nodiscard]] Entity entity(EntityId id);
	[[nodiscard]] Entity prefab(const std::string& name);
	[[nodiscard]] Entity lookup(const std::string& name);

	void destroy_entity(EntityId id);
	[[nodiscard]] bool is_valid(EntityId id) const;
	[[nodiscard]] std::string get_entity_name(EntityId id) const;
	void set_entity_name(EntityId id, const std::string& name);
	[[nodiscard]] EntityId get_parent(EntityId id) const;
	void set_parent(EntityId child, EntityId parent);
	void set_prefab(EntityId entity, EntityId prefab);
	[[nodiscard]] bool is_instance_of(EntityId entity, EntityId prefab) const;

	// ---- Component types ----

	// Registers a value type from its reflection alone, linking the component to `info`. Idempotent by name.
	EntityId register_component(const ClassInfo& info);
	// Looks the class up in ClassDB; null when it is unknown or not a value type.
	EntityId register_component(StaticString class_name);

	template <ClassDbNamed T>
	EntityId component() {
		return register_component(T::get_class_static());
	}

	// For a C++ type ClassDB does not describe; it gets no ClassInfo link.
	template <class T>
		requires(!ClassDbNamed<T>)
	EntityId component(std::string_view name) {
		if constexpr (std::is_empty_v<T>) {
			return tag(name);
		}
		else {
			return _register_component(_intern(name), sizeof(T), alignof(T), make_value_type_hooks<T>(), nullptr);
		}
	}

	// A component with no storage.
	EntityId tag(std::string_view name);

	[[nodiscard]] EntityId find_component(StaticString name) const;
	[[nodiscard]] const ClassInfo* get_component_class_info(EntityId component) const;
	[[nodiscard]] const ClassInfo* get_component_class_info(StaticString name) const;
	[[nodiscard]] std::size_t get_component_count() const { return _components.size(); }

	// ---- Components on entities, by name ----

	bool add_component(EntityId entity, StaticString name);
	[[nodiscard]] bool has_component(EntityId entity, StaticString name) const;
	void remove_component(EntityId entity, StaticString name);
	void remove_all(StaticString name);

	// Raw storage, or null. The mutable one adds the component when missing and marks it changed.
	[[nodiscard]] const void* component_data(EntityId entity, StaticString name) const;
	[[nodiscard]] void* mutable_component_data(EntityId entity, StaticString name);

	// ---- Systems and queries ----

	template <class... TComps>
	[[nodiscard]] SystemBuilder<TComps...> system(const char* name);

	template <class... TComps>
	[[nodiscard]] QueryBuilder<TComps...> query(const char* name = "");

	// ---- EcsObjects ----

	// Builds the EcsObject ClassDB registered under `class_name` and lets it declare its content, once, inside a
	// scope named after it. Null when the class is not a constructible EcsObject.
	EntityId import_object(StaticString class_name);

	template <ClassDbNamed T>
	EntityId import_object() {
		return import_object(T::get_class_static());
	}

	[[nodiscard]] bool is_object_imported(StaticString class_name) const { return _objects.contains(class_name); }
};

// One component on one entity, reflected through the ClassInfo it was registered from.
class FEATHER_API ComponentHandle : public Reflected {
	FCLASS();

	Ecs* _ecs = nullptr;
	EntityId _entity;
	EntityId _component;
	const ClassInfo* _info = nullptr;

	[[nodiscard]] const ClassInfo::Property* _find_property(std::string_view name) const;

public:
	ComponentHandle() = default;
	ComponentHandle(Ecs& ecs, EntityId entity, StaticString component);

	[[nodiscard]] bool is_valid() const;
	explicit operator bool() const { return is_valid(); }

	[[nodiscard]] EntityId get_entity() const { return _entity; }
	[[nodiscard]] EntityId get_component() const { return _component; }
	[[nodiscard]] const ClassInfo* get_class_info() const { return _info; }

	// Re-resolved per call: structural changes move an entity's storage.
	[[nodiscard]] const void* data() const;
	[[nodiscard]] void* mutable_data();

	[[nodiscard]] Variant get(std::string_view property) const;
	bool set(std::string_view property, const Variant& value);
};

namespace ecs_detail {

// `T` yields T&, `const T` is read-only, and `T*` is optional and null where absent.
template <class T>
struct TermTraits {
	using Component = std::remove_const_t<T>;
	static constexpr bool is_optional = false;
	static constexpr TermAccess access = std::is_const_v<T> ? TermAccess::In : TermAccess::InOut;

	static T& row(void* field, std::size_t i) { return static_cast<T*>(field)[i]; }
};

template <class T>
struct TermTraits<T*> {
	using Component = std::remove_const_t<T>;
	static constexpr bool is_optional = true;
	static constexpr TermAccess access = std::is_const_v<T> ? TermAccess::In : TermAccess::InOut;

	static T* row(void* field, std::size_t i) { return field ? static_cast<T*>(field) + i : nullptr; }
};

template <class T>
struct TermTraits<T&> : TermTraits<T> {};

template <class... TComps>
std::vector<Term> make_terms() {
	std::vector<Term> terms;
	terms.reserve(sizeof...(TComps));
	(terms.push_back(
			 Term {
					 TermTraits<TComps>::Component::get_class_static(),
					 TermTraits<TComps>::access,
					 TraverseFlag::Self,
					 TermTraits<TComps>::is_optional,
			 }
	 ),
	 ...);
	return terms;
}

template <class... TComps, class F, std::size_t... Is>
void invoke_each(SystemIterator& it, F& fn, std::index_sequence<Is...>) {
	// One column per term per batch, so fields resolve once rather than per row.
	void* fields[] = {
		it._field_ptr(static_cast<std::int32_t>(Is), sizeof(typename TermTraits<TComps>::Component))...
	};

	const std::size_t rows = it.count();
	for (std::size_t row = 0; row < rows; ++row) {
		if constexpr (std::is_invocable_v<
							  F&,
							  SystemIterator&,
							  std::size_t,
							  decltype(TermTraits<TComps>::row(fields[Is], row))...>)
		{
			fn(it, row, TermTraits<TComps>::row(fields[Is], row)...);
		}
		else {
			fn(it.entity_at(row), TermTraits<TComps>::row(fields[Is], row)...);
		}
	}
}

template <class F>
void invoke_each(SystemIterator& it, F& fn, std::index_sequence<>) {
	const std::size_t rows = it.count();
	for (std::size_t row = 0; row < rows; ++row) {
		if constexpr (std::is_invocable_v<F&, SystemIterator&, std::size_t>) {
			fn(it, row);
		}
		else {
			fn(it.entity_at(row));
		}
	}
}

// The callable has to outlive the builder, so it moves to the heap with a type-erased deleter.
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

// Describes one system and registers it. The pack becomes the data terms in order; with() adds data-less filters.
template <class... TComps>
class SystemBuilder {
	Ecs* _ecs = nullptr;
	SystemDesc _desc;

public:
	SystemBuilder(Ecs& ecs, const char* name) : _ecs(&ecs) {
		_desc.query.name = name ? name : "";
		_desc.query.terms = ecs_detail::make_terms<TComps...>();
		_desc.phase = ecs.phase(SystemPhase::OnUpdate);
	}

	SystemBuilder& phase(SystemPhase phase) {
		_desc.phase = _ecs->phase(phase);
		return *this;
	}

	// A phase from Ecs::create_phase.
	SystemBuilder& phase(EntityId phase) {
		_desc.phase = phase;
		return *this;
	}

	// A pipeline from Ecs::create_pipeline; the system then runs only there.
	SystemBuilder& pipeline(EntityId pipeline) {
		_desc.pipeline = pipeline;
		return *this;
	}

	SystemBuilder& multi_threaded(bool enabled = true) {
		_desc.multi_threaded = enabled;
		return *this;
	}

	SystemBuilder& tick_source(EntityId timer) {
		_desc.tick_source = timer;
		return *this;
	}

	SystemBuilder& with(StaticString component, TraverseFlag traverse = TraverseFlag::Self) {
		_desc.query.terms.push_back(Term { component, TermAccess::None, traverse, false });
		return *this;
	}

	template <ClassDbNamed T>
	SystemBuilder& with(TraverseFlag traverse = TraverseFlag::Self) {
		return with(T::get_class_static(), traverse);
	}

	// `fn` takes (Entity, components...) or (SystemIterator&, row, components...).
	template <class F>
	EntityId each(F&& fn) {
		ecs_detail::bind_each<TComps...>(
				std::forward<F>(fn), _desc.callback, _desc.callback_ctx, _desc.callback_ctx_free
		);
		_desc.run_once = false;
		return _ecs->_register_system(std::move(_desc));
	}

	// Called once per invocation with the whole batch, even when nothing matched.
	template <class F>
	EntityId run(F&& fn) {
		using Fn = std::decay_t<F>;

		_desc.callback_ctx = new Fn(std::forward<F>(fn));
		_desc.callback = [](SystemIterator& it, void* ctx) { (*static_cast<Fn*>(ctx))(it); };
		_desc.callback_ctx_free = [](void* ctx) { delete static_cast<Fn*>(ctx); };
		_desc.run_once = true;
		return _ecs->_register_system(std::move(_desc));
	}
};

// A reusable match over a list of terms. Move-only: it owns the underlying query.
template <class... TComps>
class Query {
	Ecs* _ecs = nullptr;
	void* _impl = nullptr; // ecs_query_t*

public:
	Query() = default;
	Query(Ecs& ecs, QueryDesc&& desc) : _ecs(&ecs), _impl(ecs._create_query(std::move(desc))) {}

	~Query() {
		if (_ecs && _impl) {
			_ecs->_destroy_query(_impl);
		}
	}

	Query(const Query&) = delete;
	Query& operator=(const Query&) = delete;

	Query(Query&& other) noexcept
			: _ecs(std::exchange(other._ecs, nullptr))
			, _impl(std::exchange(other._impl, nullptr)) {}

	Query& operator=(Query&& other) noexcept {
		if (this != &other) {
			if (_ecs && _impl) {
				_ecs->_destroy_query(_impl);
			}
			_ecs = std::exchange(other._ecs, nullptr);
			_impl = std::exchange(other._impl, nullptr);
		}
		return *this;
	}

	[[nodiscard]] bool is_valid() const { return _impl != nullptr; }
	explicit operator bool() const { return is_valid(); }

	// Synchronous, so unlike a system's callable this one is never copied.
	template <class F>
	void each(F&& fn) const {
		if (!_impl) {
			return;
		}
		auto&& callable = fn;
		using Fn = std::remove_reference_t<decltype(callable)>;
		_ecs->_query_each(
				_impl,
				[](SystemIterator& it, void* ctx) {
					ecs_detail::invoke_each<TComps...>(
							it, *static_cast<Fn*>(ctx), std::index_sequence_for<TComps...> {}
					);
				},
				const_cast<void*>(static_cast<const void*>(std::addressof(callable)))
		);
	}
};

template <class... TComps>
class QueryBuilder {
	Ecs* _ecs = nullptr;
	QueryDesc _desc;

public:
	QueryBuilder(Ecs& ecs, const char* name) : _ecs(&ecs) {
		_desc.name = name ? name : "";
		_desc.terms = ecs_detail::make_terms<TComps...>();
	}

	QueryBuilder& with(StaticString component, TraverseFlag traverse = TraverseFlag::Self, bool optional = false) {
		_desc.terms.push_back(Term { component, TermAccess::None, traverse, optional });
		return *this;
	}

	template <ClassDbNamed T>
	QueryBuilder& with(TraverseFlag traverse = TraverseFlag::Self, bool optional = false) {
		return with(T::get_class_static(), traverse, optional);
	}

	[[nodiscard]] Query<TComps...> build() { return Query<TComps...>(*_ecs, std::move(_desc)); }
};

template <class... TComps>
SystemBuilder<TComps...> Ecs::system(const char* name) {
	return SystemBuilder<TComps...>(*this, name);
}

template <class... TComps>
QueryBuilder<TComps...> Ecs::query(const char* name) {
	return QueryBuilder<TComps...>(*this, name);
}

template <ClassDbNamed T>
Entity& Entity::add() {
	if (_ecs) {
		_ecs->add_component(_id, T::get_class_static());
	}
	return *this;
}

template <ClassDbNamed T>
Entity& Entity::set(T value) {
	if (_ecs) {
		_ecs->_set_component(_id, _ecs->component<T>(), &value, sizeof(T), true);
	}
	return *this;
}

template <ClassDbNamed T, class... TArgs>
Entity& Entity::emplace(TArgs&&... args) {
	return set<T>(T(std::forward<TArgs>(args)...));
}

template <ClassDbNamed T>
const T* Entity::get() const {
	return _ecs ? static_cast<const T*>(_ecs->component_data(_id, T::get_class_static())) : nullptr;
}

template <ClassDbNamed T>
T* Entity::get_mutable() {
	return _ecs ? static_cast<T*>(_ecs->mutable_component_data(_id, T::get_class_static())) : nullptr;
}

template <ClassDbNamed T>
bool Entity::has() const {
	return _ecs && _ecs->has_component(_id, T::get_class_static());
}

template <ClassDbNamed T>
Entity& Entity::remove() {
	if (_ecs) {
		_ecs->remove_component(_id, T::get_class_static());
	}
	return *this;
}

} //namespace feather
