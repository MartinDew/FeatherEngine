module;

#include <flecs.h>

module feather.core;

import std;

namespace feather {

namespace {

ecs_entity_t to_flecs_phase(SystemPhase phase) {
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

std::int16_t to_flecs_inout(TermAccess access) {
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

// Up and Cascade both check the entity itself first, then walk ChildOf (not flecs' default IsA).
ecs_id_t to_flecs_traverse_flags(TraverseFlag traverse) {
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

const ValueTypeHooks& hooks_of(const ecs_type_info_t* info) {
	return *static_cast<const ValueTypeHooks*>(info->hooks.binding_ctx);
}

void hook_ctor(void* ptr, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).default_construct(ptr, static_cast<std::size_t>(count));
}

void hook_dtor(void* ptr, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).destruct(ptr, static_cast<std::size_t>(count));
}

void hook_copy(void* dst, const void* src, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).copy(dst, src, static_cast<std::size_t>(count));
}

void hook_move(void* dst, void* src, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).move(dst, src, static_cast<std::size_t>(count));
}

void hook_copy_ctor(void* dst, const void* src, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).copy_construct(dst, src, static_cast<std::size_t>(count));
}

void hook_move_ctor(void* dst, void* src, std::int32_t count, const ecs_type_info_t* info) {
	hooks_of(info).move_construct(dst, src, static_cast<std::size_t>(count));
}

ecs_flags32_t to_flecs_illegal_flags(ValueTypeIllegal illegal) {
	ecs_flags32_t flags = 0;
	if (has_flag(illegal, ValueTypeIllegal::DefaultConstruct)) {
		flags |= ECS_TYPE_HOOK_CTOR_ILLEGAL;
	}
	if (has_flag(illegal, ValueTypeIllegal::Copy)) {
		flags |= ECS_TYPE_HOOK_COPY_ILLEGAL;
	}
	if (has_flag(illegal, ValueTypeIllegal::Move)) {
		flags |= ECS_TYPE_HOOK_MOVE_ILLEGAL;
	}
	if (has_flag(illegal, ValueTypeIllegal::CopyConstruct)) {
		flags |= ECS_TYPE_HOOK_COPY_CTOR_ILLEGAL;
	}
	if (has_flag(illegal, ValueTypeIllegal::MoveConstruct)) {
		flags |= ECS_TYPE_HOOK_MOVE_CTOR_ILLEGAL;
	}
	return flags;
}

// Set on a component's own entity: the reflection it was registered from.
struct ComponentClassInfo {
	const ClassInfo* info = nullptr;
};

// Set on an imported EcsObject's module entity, which owns the instance for the world's lifetime.
struct EcsObjectInstance {
	std::unique_ptr<EcsObject> object;
};

void free_hooks(void* hooks) {
	delete static_cast<ValueTypeHooks*>(hooks);
}

// flecs' own flecs_entity_compare, which is internal: systems in one phase run in declaration order.
int compare_entities(ecs_entity_t e1, const void*, ecs_entity_t e2, const void*) {
	return (e1 > e2) - (e1 < e2);
}

struct SystemContext {
	Ecs* ecs = nullptr;
	SystemCallback callback = nullptr;
	void* user_ctx = nullptr;
	ContextDeleter user_ctx_free = nullptr;
};

void system_ctx_free(void* ptr) {
	auto* context = static_cast<SystemContext*>(ptr);
	if (context->user_ctx_free) {
		context->user_ctx_free(context->user_ctx);
	}
	delete context;
}

void system_trampoline(ecs_iter_t* it) {
	auto* context = static_cast<SystemContext*>(it->callback_ctx);
	SystemIterator iterator(*context->ecs, it);
	context->callback(iterator, context->user_ctx);
}

// Clears EcsIterIsValid as flecs' own C++ run() path does. flecs frees a query that matches nothing itself;
// any other iterator the callback did not exhaust is ours to fini.
void system_run_trampoline(ecs_iter_t* it) {
	auto* context = static_cast<SystemContext*>(it->run_ctx);
	it->flags &= ~EcsIterIsValid;
	SystemIterator iterator(*context->ecs, it);
	context->callback(iterator, context->user_ctx);
	const bool flecs_finis = it->query && (it->query->flags & EcsQueryMatchNothing);
	if (!flecs_finis && !iterator.is_finished()) {
		iterator.fini();
	}
}

// flecs wants NUL-terminated names; the common short ones are terminated on the stack.
template <class F>
decltype(auto) with_terminated(std::string_view name, F&& fn) {
	constexpr std::size_t buffer_size = 128;
	if (name.size() < buffer_size) {
		std::array<char, buffer_size> buffer;
		std::memcpy(buffer.data(), name.data(), name.size());
		buffer[name.size()] = '\0';
		return fn(static_cast<const char*>(buffer.data()));
	}
	const std::string terminated(name);
	return fn(terminated.c_str());
}

std::atomic<std::uint32_t> next_component_slot { 0 };

} //namespace

struct Ecs::Impl {
	flecs::world world;
	ecs_entity_t class_info_component = 0;
	ecs_entity_t object_component = 0;

	// Component ids by ComponentSlot, filled on first typed use. A memo of the name lookup, not a second registry.
	std::vector<ecs_entity_t> slot_ids;

	Impl() {
		class_info_component = world.component<ComponentClassInfo>("FeatherClassInfo").id();
		object_component = world.component<EcsObjectInstance>("FeatherEcsObject").id();
	}

	// A root-level entity named `name` carrying `marker`, or 0.
	ecs_entity_t find_root(std::string_view name, ecs_id_t marker) const {
		const ecs_entity_t found =
				with_terminated(name, [this](const char* terminated) { return ecs_lookup_child(flecs_ptr(), 0, terminated); });
		return found && ecs_has_id(flecs_ptr(), found, marker) ? found : 0;
	}

	ecs_world_t* flecs_ptr() const { return world.c_ptr(); }
};

Ecs::Ecs() : _impl(std::make_unique<Impl>()) {
}

// The world owns every EcsObject instance, so tearing it down destroys them too.
Ecs::~Ecs() = default;

// ---- Simulation ----

bool Ecs::progress(double delta) {
	return _impl->world.progress(static_cast<float>(delta));
}

EntityId Ecs::create_timer(double interval) {
	return EntityId(_impl->world.timer().interval(static_cast<float>(interval)).id());
}

float Ecs::delta_time() const {
	return static_cast<float>(_impl->world.delta_time());
}

void Ecs::enable_rest_api() {
	_impl->world.set<flecs::Rest>({});
}

EntityId Ecs::phase(SystemPhase phase) const {
	return EntityId(to_flecs_phase(phase));
}

PipelineBuilder Ecs::pipeline(const std::string& name) {
	return PipelineBuilder(*this, name);
}

EntityId Ecs::_register_pipeline(
		const std::string& name,
		std::span<const StaticString> with,
		std::span<const StaticString> without
) {
	ecs_world_t* world = _impl->flecs_ptr();
	ecs_entity_desc_t entity_desc {};
	entity_desc.name = name.empty() ? nullptr : name.c_str();

	ecs_pipeline_desc_t desc {};
	desc.entity = ecs_entity_init(world, &entity_desc);
	desc.query.order_by_callback = compare_entities;

	// The same terms as flecs' BuiltinPipeline.
	ecs_term_t* terms = desc.query.terms;
	std::size_t term = 0;
	terms[term++].id = EcsSystem;

	terms[term].id = EcsPhase;
	terms[term].src.id = EcsCascade;
	terms[term++].trav = EcsDependsOn;

	terms[term].id = ecs_dependson(EcsOnStart);
	terms[term].trav = EcsDependsOn;
	terms[term++].oper = EcsNot;

	terms[term].id = EcsDisabled;
	terms[term].src.id = EcsUp;
	terms[term].trav = EcsDependsOn;
	terms[term++].oper = EcsNot;

	terms[term].id = EcsDisabled;
	terms[term].src.id = EcsUp;
	terms[term].trav = EcsChildOf;
	terms[term++].oper = EcsNot;

	if (term + with.size() + without.size() > FLECS_TERM_COUNT_MAX) {
		fassert(false, "Too many terms for a pipeline");
		return {};
	}
	for (const StaticString& component : with) {
		const EntityId id = register_component(component);
		fassert(!id.is_null(), "A pipeline named a component the ECS does not know");
		terms[term].id = id.raw();
		terms[term++].inout = EcsInOutNone;
	}
	for (const StaticString& component : without) {
		const EntityId id = register_component(component);
		fassert(!id.is_null(), "A pipeline named a component the ECS does not know");
		terms[term].id = id.raw();
		terms[term].oper = EcsNot;
		terms[term++].inout = EcsInOutNone;
	}
	return EntityId(ecs_pipeline_init(world, &desc));
}

EntityId Ecs::get_pipeline() const {
	return EntityId(ecs_get_pipeline(_impl->flecs_ptr()));
}

void Ecs::set_pipeline(EntityId pipeline) {
	ecs_set_pipeline(_impl->flecs_ptr(), pipeline.raw());
}

void Ecs::run_pipeline(EntityId pipeline, double delta) {
	ecs_run_pipeline(_impl->flecs_ptr(), pipeline.raw(), static_cast<ecs_ftime_t>(delta == 0.0 ? delta_time() : delta));
}

EntityId Ecs::create_phase(const std::string& name, EntityId runs_after) {
	ecs_world_t* world = _impl->flecs_ptr();
	ecs_entity_desc_t desc {};
	desc.name = name.empty() ? nullptr : name.c_str();
	const ecs_entity_t created = ecs_entity_init(world, &desc);
	ecs_add_id(world, created, EcsPhase);
	// The pipeline orders phases by walking DependsOn; that chain is the only ordering a phase has.
	if (!runs_after.is_null()) {
		ecs_add_pair(world, created, EcsDependsOn, runs_after.raw());
	}
	return EntityId(created);
}

// ---- Entities ----

Entity Ecs::entity(const std::string& name) {
	const flecs::entity created = name.empty() ? _impl->world.entity() : _impl->world.entity(name.c_str());
	return { *this, EntityId(created.id()) };
}

Entity Ecs::entity(EntityId id) {
	return { *this, id };
}

Entity Ecs::prefab(const std::string& name) {
	return { *this, EntityId(_impl->world.prefab(name.c_str()).id()) };
}

Entity Ecs::lookup(const std::string& name) {
	return { *this, EntityId(_impl->world.lookup(name.c_str()).id()) };
}

void Ecs::destroy_entity(EntityId id) {
	ecs_delete(_impl->flecs_ptr(), id.raw());
}

bool Ecs::is_valid(EntityId id) const {
	return !id.is_null() && ecs_is_valid(_impl->flecs_ptr(), id.raw());
}

std::string Ecs::get_entity_name(EntityId id) const {
	const char* name = ecs_get_name(_impl->flecs_ptr(), id.raw());
	return name ? std::string(name) : std::string {};
}

void Ecs::set_entity_name(EntityId id, const std::string& name) {
	ecs_set_name(_impl->flecs_ptr(), id.raw(), name.c_str());
}

EntityId Ecs::get_parent(EntityId id) const {
	return EntityId(ecs_get_target(_impl->flecs_ptr(), id.raw(), EcsChildOf, 0));
}

void Ecs::set_parent(EntityId child, EntityId parent) {
	ecs_add_pair(_impl->flecs_ptr(), child.raw(), EcsChildOf, parent.raw());
}

void Ecs::set_prefab(EntityId entity, EntityId prefab) {
	ecs_add_pair(_impl->flecs_ptr(), entity.raw(), EcsIsA, prefab.raw());
}

bool Ecs::is_instance_of(EntityId entity, EntityId prefab) const {
	return ecs_has_pair(_impl->flecs_ptr(), entity.raw(), EcsIsA, prefab.raw());
}

// ---- Component types ----

EntityId Ecs::register_component(const ClassInfo& info) {
	if (const EntityId existing = find_component(info.name); !existing.is_null()) {
		return existing;
	}
	// Only a value type's ClassInfo describes storage; a Reflected object lives on the heap, not in a column.
	if (!info.is_value_type || info.size == 0) {
		return {};
	}
	return _register_component(
			info.name.str(), info.is_empty ? 0 : info.size, info.is_empty ? 0 : info.alignment, info.value_hooks, &info
	);
}

EntityId Ecs::register_component(StaticString class_name) {
	if (const EntityId existing = find_component(class_name); !existing.is_null()) {
		return existing;
	}
	const ClassInfo* info = ClassDB::get_class_info(class_name);
	return info ? register_component(*info) : EntityId {};
}

EntityId Ecs::tag(std::string_view name) {
	return _register_component(name, 0, 0, {}, nullptr);
}

EntityId Ecs::_register_component(
		std::string_view name,
		std::size_t size,
		std::size_t alignment,
		const ValueTypeHooks& hooks,
		const ClassInfo* info
) {
	if (const EntityId existing = find_component(StaticString(name)); !existing.is_null()) {
		return existing;
	}

	// Always at the root, so a name resolves the same regardless of which EcsObject scope is open.
	ecs_world_t* world = _impl->flecs_ptr();
	const ecs_entity_t previous_scope = ecs_set_scope(world, 0);

	const std::string terminated(name);
	ecs_entity_desc_t entity_desc {};
	entity_desc.name = terminated.c_str();
	entity_desc.symbol = terminated.c_str();

	ecs_component_desc_t desc {};
	desc.entity = ecs_entity_init(world, &entity_desc);
	desc.type.size = static_cast<ecs_size_t>(size);
	desc.type.alignment = static_cast<ecs_size_t>(alignment);
	if (size > 0) {
		auto* owned = new ValueTypeHooks(hooks);
		desc.type.hooks.binding_ctx = owned;
		desc.type.hooks.binding_ctx_free = free_hooks;
		desc.type.hooks.flags = to_flecs_illegal_flags(owned->illegal);
		// flecs zero-fills when ctor is null; flecs_default_ctor keeps that while still being an explicit hook.
		if (!has_flag(owned->illegal, ValueTypeIllegal::DefaultConstruct)) {
			desc.type.hooks.ctor = owned->default_construct ? hook_ctor : flecs_default_ctor;
		}
		desc.type.hooks.dtor = owned->destruct ? hook_dtor : nullptr;
		desc.type.hooks.copy = owned->copy ? hook_copy : nullptr;
		desc.type.hooks.move = owned->move ? hook_move : nullptr;
		desc.type.hooks.copy_ctor = owned->copy_construct ? hook_copy_ctor : nullptr;
		desc.type.hooks.move_ctor = owned->move_construct ? hook_move_ctor : nullptr;
	}

	const EntityId id { ecs_component_init(world, &desc) };
	ecs_set_scope(world, previous_scope);

	if (!id.is_null() && info) {
		const ComponentClassInfo link { info };
		ecs_set_id(world, id.raw(), _impl->class_info_component, sizeof(ComponentClassInfo), &link);
	}
	return id;
}

EntityId Ecs::find_component(StaticString name) const {
	return EntityId(_impl->find_root(name.str(), ecs_id(EcsComponent)));
}

const ClassInfo* Ecs::get_component_class_info(EntityId component) const {
	if (!is_valid(component)) {
		return nullptr;
	}
	const auto* link = static_cast<const ComponentClassInfo*>(
			ecs_get_id(_impl->flecs_ptr(), component.raw(), _impl->class_info_component)
	);
	return link ? link->info : nullptr;
}

const ClassInfo* Ecs::get_component_class_info(StaticString name) const {
	return get_component_class_info(find_component(name));
}

// ---- Components on entities ----

bool Ecs::add_component(EntityId entity, StaticString name) {
	return _add(entity, register_component(name));
}

bool Ecs::has_component(EntityId entity, StaticString name) const {
	return _has(entity, find_component(name));
}

void Ecs::remove_component(EntityId entity, StaticString name) {
	_remove(entity, find_component(name));
}

bool Ecs::_add(EntityId entity, EntityId component) {
	if (component.is_null() || !is_valid(entity)) {
		return false;
	}
	ecs_add_id(_impl->flecs_ptr(), entity.raw(), component.raw());
	return true;
}

bool Ecs::_has(EntityId entity, EntityId component) const {
	return !component.is_null() && is_valid(entity) && ecs_has_id(_impl->flecs_ptr(), entity.raw(), component.raw());
}

void Ecs::_remove(EntityId entity, EntityId component) {
	if (!component.is_null() && is_valid(entity)) {
		ecs_remove_id(_impl->flecs_ptr(), entity.raw(), component.raw());
	}
}

std::uint32_t Ecs::_allocate_slot() {
	return next_component_slot.fetch_add(1, std::memory_order_relaxed);
}

// Like flecs' own lazy type registration, filling a slot is not safe from several threads at once.
EntityId Ecs::_component_id(std::uint32_t slot, StaticString name) {
	std::vector<ecs_entity_t>& ids = _impl->slot_ids;
	if (slot < ids.size() && ids[slot] != 0) {
		return EntityId(ids[slot]);
	}
	const EntityId id = register_component(name);
	if (!id.is_null()) {
		if (slot >= ids.size()) {
			ids.resize(slot + 1, 0);
		}
		ids[slot] = id.raw();
	}
	return id;
}

void Ecs::remove_all(StaticString name) {
	if (const EntityId component = find_component(name); !component.is_null()) {
		ecs_remove_all(_impl->flecs_ptr(), component.raw());
	}
}

const void* Ecs::component_data(EntityId entity, StaticString name) const {
	return _component_data(entity, find_component(name));
}

void* Ecs::mutable_component_data(EntityId entity, StaticString name) {
	return _mutable_component_data(entity, register_component(name));
}

// ecs_get_id is null when the entity lacks the component, so no separate has check.
const void* Ecs::_component_data(EntityId entity, EntityId component) const {
	if (component.is_null() || !is_valid(entity)) {
		return nullptr;
	}
	return ecs_get_id(_impl->flecs_ptr(), entity.raw(), component.raw());
}

void* Ecs::_mutable_component_data(EntityId entity, EntityId component) {
	if (component.is_null() || !is_valid(entity)) {
		return nullptr;
	}
	const ecs_type_info_t* type_info = ecs_get_type_info(_impl->flecs_ptr(), component.raw());
	if (!type_info || type_info->size <= 0) {
		return nullptr;
	}
	return ecs_ensure_id(_impl->flecs_ptr(), entity.raw(), component.raw(), static_cast<std::size_t>(type_info->size));
}

void Ecs::_modified(EntityId entity, EntityId component) {
	if (!component.is_null() && is_valid(entity) && ecs_has_id(_impl->flecs_ptr(), entity.raw(), component.raw())) {
		ecs_modified_id(_impl->flecs_ptr(), entity.raw(), component.raw());
	}
}

void Ecs::modified(EntityId entity, StaticString name) {
	_modified(entity, find_component(name));
}

bool Ecs::_set_component(EntityId entity, EntityId component, void* value, std::size_t size, bool move) {
	if (component.is_null() || !is_valid(entity)) {
		return false;
	}

	ecs_world_t* world = _impl->flecs_ptr();
	const ecs_type_info_t* type_info = ecs_get_type_info(world, component.raw());
	if (!type_info || type_info->size <= 0) {
		ecs_add_id(world, entity.raw(), component.raw());
		return true;
	}
	if (static_cast<std::size_t>(type_info->size) != size) {
		return false;
	}

	// Decided before emplacing: storage flecs hands back unconstructed must be constructed.
	const ValueTypeHooks& hooks = hooks_of(type_info);
	const bool exists = ecs_has_id(world, entity.raw(), component.raw());
	const bool use_move =
			move && !has_flag(hooks.illegal, exists ? ValueTypeIllegal::Move : ValueTypeIllegal::MoveConstruct);
	if (!use_move && has_flag(hooks.illegal, exists ? ValueTypeIllegal::Copy : ValueTypeIllegal::CopyConstruct)) {
		fassert(false, "Setting a component its type can neither copy nor move");
		return false;
	}

	bool is_new = false;
	void* destination = ecs_emplace_id(world, entity.raw(), component.raw(), size, &is_new);
	if (!destination) {
		return false;
	}
	fassert(is_new != exists, "Component storage changed state while being set");

	using Transfer = void (*)(void*, void*, std::size_t);
	using CopyTransfer = void (*)(void*, const void*, std::size_t);
	const Transfer move_op = is_new ? hooks.move_construct : hooks.move;
	const CopyTransfer copy_op = is_new ? hooks.copy_construct : hooks.copy;
	if (use_move && move_op) {
		move_op(destination, value, 1);
	}
	else if (!use_move && copy_op) {
		copy_op(destination, value, 1);
	}
	else {
		// A null hook that is not illegal is a trivial one.
		std::memcpy(destination, value, size);
	}
	ecs_modified_id(world, entity.raw(), component.raw());
	return true;
}

// ---- Systems and queries ----

namespace {

bool fill_query_desc(Ecs& ecs, ecs_query_desc_t& out, const QueryDesc& desc) {
	if (desc.terms.size() > FLECS_TERM_COUNT_MAX) {
		return false;
	}
	for (std::size_t i = 0; i < desc.terms.size(); ++i) {
		const Term& term = desc.terms[i];
		// Registering, not just finding: a system may name a component nothing has instantiated yet.
		const EntityId component = ecs.register_component(term.component);
		if (component.is_null()) {
			std::println(std::cerr, "ECS: '{}' in '{}' is not a registered component", term.component, desc.name);
			return false;
		}

		ecs_term_t& out_term = out.terms[i];
		out_term.id = component.raw();
		out_term.inout = to_flecs_inout(term.access);
		out_term.oper = term.optional ? EcsOptional : EcsAnd;
		out_term.src.id = to_flecs_traverse_flags(term.traverse);
		out_term.trav = term.traverse == TraverseFlag::Self ? 0 : EcsChildOf;
	}
	return true;
}

} //namespace

EntityId Ecs::_register_system(SystemDesc&& desc) {
	ecs_system_desc_t system_desc {};
	if (!fill_query_desc(*this, system_desc.query, desc.query)) {
		fassert(false, "A system named a component the ECS does not know");
		if (desc.callback_ctx_free) {
			desc.callback_ctx_free(desc.callback_ctx);
		}
		return {};
	}

	ecs_entity_desc_t entity_desc {};
	entity_desc.name = desc.query.name.empty() ? nullptr : desc.query.name.c_str();
	system_desc.entity = ecs_entity_init(_impl->flecs_ptr(), &entity_desc);

	auto* context = new SystemContext { this, desc.callback, desc.callback_ctx, desc.callback_ctx_free };
	if (desc.run_once) {
		system_desc.run = system_run_trampoline;
		system_desc.run_ctx = context;
		system_desc.run_ctx_free = system_ctx_free;
	}
	else {
		system_desc.callback = system_trampoline;
		system_desc.callback_ctx = context;
		system_desc.callback_ctx_free = system_ctx_free;
	}
	// A null phase keeps the system out of every pipeline.
	system_desc.phase = desc.phase.raw();
	system_desc.multi_threaded = desc.multi_threaded;
	system_desc.tick_source = desc.tick_source.raw();

	return EntityId(ecs_system_init(_impl->flecs_ptr(), &system_desc));
}

void* Ecs::_create_query(QueryDesc&& desc) {
	ecs_query_desc_t query_desc {};
	if (!fill_query_desc(*this, query_desc, desc)) {
		fassert(false, "A query named a component the ECS does not know");
		return nullptr;
	}
	query_desc.cache_kind = desc.cached ? EcsQueryCacheAuto : EcsQueryCacheDefault;
	return ecs_query_init(_impl->flecs_ptr(), &query_desc);
}

void Ecs::_destroy_query(void* query) const {
	if (query) {
		ecs_query_fini(static_cast<ecs_query_t*>(query));
	}
}

void Ecs::_query_each(void* query, SystemCallback callback, void* ctx) {
	if (!query || !callback) {
		return;
	}
	ecs_iter_t it = ecs_query_iter(_impl->flecs_ptr(), static_cast<ecs_query_t*>(query));
	while (ecs_query_next(&it)) {
		SystemIterator iterator(*this, &it);
		callback(iterator, ctx);
	}
}

// ---- EcsObjects ----

EntityId Ecs::import_object(StaticString class_name) {
	if (const ecs_entity_t existing = _impl->find_root(class_name.str(), _impl->object_component)) {
		return EntityId(existing);
	}

	const ClassInfo* info = ClassDB::get_class_info(class_name);
	if (!info || !info->object_create_func || !ClassDB::has_parent(info->name, EcsObject::get_class_static())) {
		return {};
	}
	std::unique_ptr<EcsObject> object = ClassDB::create_object<EcsObject>(class_name);
	if (!object) {
		return {};
	}
	EcsObject* instance = object.get();

	// A module entity scopes everything the object declares, the way flecs namespaces its own modules.
	flecs::world& world = _impl->world;
	const ecs_entity_t previous_scope = ecs_set_scope(world.c_ptr(), 0);
	const std::string name(class_name.str());
	flecs::entity scope = world.entity(name.c_str());
	ecs_set_scope(world.c_ptr(), previous_scope);
	scope.add(flecs::Module);
	// Set before on_import, so an object importing another that imports it back finds it here.
	scope.set(EcsObjectInstance { std::move(object) });

	std::println("Importing ECS object {}", class_name);
	const ecs_entity_t outer_scope = ecs_set_scope(world.c_ptr(), scope.id());
	instance->on_import(*this);
	ecs_set_scope(world.c_ptr(), outer_scope);
	return EntityId(scope.id());
}

bool Ecs::is_object_imported(StaticString class_name) const {
	return _impl->find_root(class_name.str(), _impl->object_component) != 0;
}

// ---- Entity ----

bool Entity::is_valid() const {
	return _ecs && _ecs->is_valid(_id);
}

std::string Entity::get_name() const {
	return is_valid() ? _ecs->get_entity_name(_id) : std::string {};
}

void Entity::set_name(const std::string& name) {
	if (is_valid()) {
		_ecs->set_entity_name(_id, name);
	}
}

void Entity::destroy() {
	if (is_valid()) {
		_ecs->destroy_entity(_id);
	}
	_id = {};
}

Entity& Entity::child_of(const Entity& parent) {
	if (is_valid()) {
		_ecs->set_parent(_id, parent._id);
	}
	return *this;
}

Entity& Entity::is_a(const Entity& prefab) {
	if (is_valid()) {
		_ecs->set_prefab(_id, prefab._id);
	}
	return *this;
}

Entity Entity::get_parent() const {
	return is_valid() ? Entity { *_ecs, _ecs->get_parent(_id) } : Entity {};
}

bool Entity::is_descendant_of(const Entity& ancestor) const {
	if (!is_valid() || !ancestor.is_valid()) {
		return false;
	}
	for (EntityId current = _id; !current.is_null(); current = _ecs->get_parent(current)) {
		if (current == ancestor._id) {
			return true;
		}
	}
	return false;
}

bool Entity::is_instance_of(const Entity& prefab) const {
	return is_valid() && _ecs->is_instance_of(_id, prefab._id);
}

Entity& Entity::add_component(StaticString component) {
	if (_ecs) {
		_ecs->add_component(_id, component);
	}
	return *this;
}

bool Entity::has_component(StaticString component) const {
	return _ecs && _ecs->has_component(_id, component);
}

Entity& Entity::modified(StaticString component) {
	if (_ecs) {
		_ecs->modified(_id, component);
	}
	return *this;
}

Entity& Entity::remove_component(StaticString component) {
	if (_ecs) {
		_ecs->remove_component(_id, component);
	}
	return *this;
}

ComponentHandle Entity::component(StaticString component) const {
	return _ecs ? ComponentHandle(*_ecs, _id, component) : ComponentHandle {};
}

// ---- SystemIterator ----

std::size_t SystemIterator::count() const {
	return _impl ? static_cast<std::size_t>(static_cast<ecs_iter_t*>(_impl)->count) : 0;
}

float SystemIterator::delta_time() const {
	return _impl ? static_cast<float>(static_cast<ecs_iter_t*>(_impl)->delta_time) : 0.0f;
}

Entity SystemIterator::entity_at(std::size_t row) const {
	const auto* it = static_cast<ecs_iter_t*>(_impl);
	if (!it || !_ecs || row >= static_cast<std::size_t>(it->count)) {
		return {};
	}
	return { *_ecs, EntityId(it->entities[row]) };
}

bool SystemIterator::next() {
	auto* it = static_cast<ecs_iter_t*>(_impl);
	if (!it || _finished) {
		return false;
	}
	// A false return has already released the iterator.
	_finished = !ecs_iter_next(it);
	return !_finished;
}

void SystemIterator::fini() {
	if (_impl && !_finished) {
		ecs_iter_fini(static_cast<ecs_iter_t*>(_impl));
	}
	_finished = true;
}

void* SystemIterator::_field_ptr(std::int32_t term, std::size_t size) const {
	auto* it = static_cast<ecs_iter_t*>(_impl);
	if (!it || term < 0 || term >= it->field_count || !ecs_field_is_set(it, static_cast<std::int8_t>(term))) {
		return nullptr;
	}
	return ecs_field_w_size(it, size, static_cast<std::int8_t>(term));
}

// ---- ComponentHandle ----

ComponentHandle::ComponentHandle(Ecs& ecs, EntityId entity, StaticString component)
		: _ecs(&ecs)
		, _entity(entity)
		, _component(ecs.find_component(component)) {
	_info = ecs.get_component_class_info(_component);
}

const ClassInfo::Property* ComponentHandle::_find_property(std::string_view name) const {
	if (!_info) {
		return nullptr;
	}
	for (const ClassInfo::Property& property : _info->properties) {
		if (property.name == StaticString(name)) {
			return &property;
		}
	}
	return nullptr;
}

bool ComponentHandle::is_valid() const {
	return _ecs && !_component.is_null() && _ecs->is_valid(_entity) &&
			ecs_has_id(_ecs->_impl->flecs_ptr(), _entity.raw(), _component.raw());
}

const void* ComponentHandle::data() const {
	return _ecs ? _ecs->_component_data(_entity, _component) : nullptr;
}

void* ComponentHandle::mutable_data() {
	return _ecs ? _ecs->_mutable_component_data(_entity, _component) : nullptr;
}

Variant ComponentHandle::get(std::string_view property) const {
	const ClassInfo::Property* found = _find_property(property);
	const void* storage = data();
	if (!found || !found->getter || !storage) {
		return {};
	}
	// A value type's accessors take the instance itself, not a Reflected*.
	return found->getter(const_cast<void*>(storage));
}

bool ComponentHandle::set(std::string_view property, const Variant& value) {
	const ClassInfo::Property* found = _find_property(property);
	if (!found || !found->setter) {
		return false;
	}
	void* storage = mutable_data();
	if (!storage) {
		return false;
	}
	found->setter(storage, value);
	_ecs->_modified(_entity, _component);
	return true;
}

} //namespace feather
