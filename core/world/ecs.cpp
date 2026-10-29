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

// Stored on each component entity, so flecs-side tooling can also reach a component's reflection.
struct ComponentClassInfo {
	const ClassInfo* info = nullptr;
};

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

// Clears EcsIterIsValid rather than calling ecs_iter_fini, as flecs' own C++ run() path does.
void system_run_trampoline(ecs_iter_t* it) {
	auto* context = static_cast<SystemContext*>(it->run_ctx);
	it->flags &= ~EcsIterIsValid;
	SystemIterator iterator(*context->ecs, it);
	context->callback(iterator, context->user_ctx);
}

} //namespace

struct Ecs::Impl {
	// Declared before the world: component dtors run during world teardown and read these.
	std::deque<ValueTypeHooks> hooks;
	flecs::world world;
	ecs_entity_t class_info_component = 0;

	Impl() {
		class_info_component = world.component<ComponentClassInfo>("FeatherComponentClassInfo").id();
	}

	ecs_world_t* c() const { return world.c_ptr(); }
};

StaticString Ecs::_intern(std::string_view name) {
	// Only ever grows, so every view handed out stays valid for the process.
	static auto* names = new std::deque<std::string>();
	for (const std::string& existing : *names) {
		if (existing == name) {
			return StaticString(existing);
		}
	}
	names->emplace_back(name);
	return StaticString(names->back());
}

Ecs::Ecs() : _impl(std::make_unique<Impl>()) {}

// EcsObjects go first: they may hold handles into the world.
Ecs::~Ecs() {
	_object_instances.clear();
	_impl.reset();
}

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

EntityId Ecs::create_phase(const std::string& name, EntityId runs_after) {
	ecs_world_t* world = _impl->c();
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
	ecs_delete(_impl->c(), id.raw());
}

bool Ecs::is_valid(EntityId id) const {
	return !id.is_null() && ecs_is_valid(_impl->c(), id.raw());
}

std::string Ecs::get_entity_name(EntityId id) const {
	const char* name = ecs_get_name(_impl->c(), id.raw());
	return name ? std::string(name) : std::string {};
}

void Ecs::set_entity_name(EntityId id, const std::string& name) {
	ecs_set_name(_impl->c(), id.raw(), name.c_str());
}

EntityId Ecs::get_parent(EntityId id) const {
	return EntityId(ecs_get_target(_impl->c(), id.raw(), EcsChildOf, 0));
}

void Ecs::set_parent(EntityId child, EntityId parent) {
	ecs_add_pair(_impl->c(), child.raw(), EcsChildOf, parent.raw());
}

void Ecs::set_prefab(EntityId entity, EntityId prefab) {
	ecs_add_pair(_impl->c(), entity.raw(), EcsIsA, prefab.raw());
}

bool Ecs::is_instance_of(EntityId entity, EntityId prefab) const {
	return ecs_has_pair(_impl->c(), entity.raw(), EcsIsA, prefab.raw());
}

// ---- Component types ----

EntityId Ecs::register_component(const ClassInfo& info) {
	if (auto it = _components.find(info.name); it != _components.end()) {
		return it->second;
	}
	// Only a value type's ClassInfo describes storage; a Reflected object lives on the heap, not in a column.
	if (!info.is_value_type || info.size == 0) {
		return {};
	}
	const StaticString name = _intern(info.name.str());
	if (info.is_empty) {
		const EntityId id = tag(name.str());
		if (!id.is_null()) {
			_link_class_info(id, info);
		}
		return id;
	}
	return _register_component(name, info.size, info.alignment, info.value_hooks, &info);
}

EntityId Ecs::register_component(StaticString class_name) {
	if (auto it = _components.find(class_name); it != _components.end()) {
		return it->second;
	}
	const ClassInfo* info = ClassDB::get_class_info(class_name);
	return info ? register_component(*info) : EntityId {};
}

EntityId Ecs::tag(std::string_view name) {
	const StaticString interned = _intern(name);
	if (auto it = _components.find(interned); it != _components.end()) {
		return it->second;
	}

	// Always at the root, so a name resolves the same regardless of which EcsObject scope is open.
	const ecs_entity_t previous_scope = ecs_set_scope(_impl->c(), 0);
	ecs_entity_desc_t desc {};
	desc.name = interned.data();
	const EntityId id { ecs_entity_init(_impl->c(), &desc) };
	ecs_set_scope(_impl->c(), previous_scope);

	if (!id.is_null()) {
		_components[interned] = id;
	}
	return id;
}

EntityId Ecs::_register_component(
		StaticString name,
		std::size_t size,
		std::size_t alignment,
		const ValueTypeHooks& hooks,
		const ClassInfo* info
) {
	if (auto it = _components.find(name); it != _components.end()) {
		return it->second;
	}

	ecs_world_t* world = _impl->c();
	const ecs_entity_t previous_scope = ecs_set_scope(world, 0);

	ecs_entity_desc_t entity_desc {};
	entity_desc.name = name.data();
	entity_desc.symbol = name.data();

	const ValueTypeHooks* kept = &_impl->hooks.emplace_back(hooks);

	ecs_component_desc_t desc {};
	desc.entity = ecs_entity_init(world, &entity_desc);
	desc.type.size = static_cast<ecs_size_t>(size);
	desc.type.alignment = static_cast<ecs_size_t>(alignment);
	desc.type.name = name.data();
	desc.type.hooks.binding_ctx = const_cast<ValueTypeHooks*>(kept);
	// flecs zero-fills when ctor is null; flecs_default_ctor keeps that while still being an explicit hook.
	desc.type.hooks.ctor = kept->default_construct ? hook_ctor : flecs_default_ctor;
	desc.type.hooks.dtor = kept->destruct ? hook_dtor : nullptr;
	desc.type.hooks.copy = kept->copy ? hook_copy : nullptr;
	desc.type.hooks.move = kept->move ? hook_move : nullptr;

	const EntityId id { ecs_component_init(world, &desc) };
	ecs_set_scope(world, previous_scope);
	if (id.is_null()) {
		return {};
	}

	_components[name] = id;
	if (info) {
		_link_class_info(id, *info);
	}
	return id;
}

void Ecs::_link_class_info(EntityId component, const ClassInfo& info) {
	_component_infos[component.raw()] = &info;
	const ComponentClassInfo link { &info };
	ecs_set_id(_impl->c(), component.raw(), _impl->class_info_component, sizeof(ComponentClassInfo), &link);
}

EntityId Ecs::find_component(StaticString name) const {
	auto it = _components.find(name);
	return it == _components.end() ? EntityId {} : it->second;
}

const ClassInfo* Ecs::get_component_class_info(EntityId component) const {
	auto it = _component_infos.find(component.raw());
	return it == _component_infos.end() ? nullptr : it->second;
}

const ClassInfo* Ecs::get_component_class_info(StaticString name) const {
	return get_component_class_info(find_component(name));
}

// ---- Components on entities ----

bool Ecs::add_component(EntityId entity, StaticString name) {
	const EntityId component = register_component(name);
	if (component.is_null() || !is_valid(entity)) {
		return false;
	}
	ecs_add_id(_impl->c(), entity.raw(), component.raw());
	return true;
}

bool Ecs::has_component(EntityId entity, StaticString name) const {
	const EntityId component = find_component(name);
	return !component.is_null() && is_valid(entity) && ecs_has_id(_impl->c(), entity.raw(), component.raw());
}

void Ecs::remove_component(EntityId entity, StaticString name) {
	if (const EntityId component = find_component(name); !component.is_null() && is_valid(entity)) {
		ecs_remove_id(_impl->c(), entity.raw(), component.raw());
	}
}

void Ecs::remove_all(StaticString name) {
	if (const EntityId component = find_component(name); !component.is_null()) {
		ecs_remove_all(_impl->c(), component.raw());
	}
}

const void* Ecs::component_data(EntityId entity, StaticString name) const {
	return _component_data(entity, find_component(name));
}

void* Ecs::mutable_component_data(EntityId entity, StaticString name) {
	return _mutable_component_data(entity, register_component(name));
}

const void* Ecs::_component_data(EntityId entity, EntityId component) const {
	if (component.is_null() || !is_valid(entity) || !ecs_has_id(_impl->c(), entity.raw(), component.raw())) {
		return nullptr;
	}
	return ecs_get_id(_impl->c(), entity.raw(), component.raw());
}

void* Ecs::_mutable_component_data(EntityId entity, EntityId component) {
	if (component.is_null() || !is_valid(entity)) {
		return nullptr;
	}
	const ecs_type_info_t* type_info = ecs_get_type_info(_impl->c(), component.raw());
	if (!type_info || type_info->size <= 0) {
		return nullptr;
	}
	void* data = ecs_ensure_id(_impl->c(), entity.raw(), component.raw(), static_cast<std::size_t>(type_info->size));
	if (data) {
		ecs_modified_id(_impl->c(), entity.raw(), component.raw());
	}
	return data;
}

bool Ecs::_set_component(EntityId entity, EntityId component, void* value, std::size_t size, bool move) {
	if (component.is_null() || !is_valid(entity)) {
		return false;
	}

	ecs_world_t* world = _impl->c();
	const ecs_type_info_t* type_info = ecs_get_type_info(world, component.raw());
	if (!type_info || type_info->size <= 0) {
		ecs_add_id(world, entity.raw(), component.raw());
		return true;
	}
	if (static_cast<std::size_t>(type_info->size) != size) {
		return false;
	}

	// ecs_ensure_id hands back constructed storage, so this is an assignment and must go through the hooks.
	void* destination = ecs_ensure_id(world, entity.raw(), component.raw(), size);
	if (!destination) {
		return false;
	}
	const ValueTypeHooks& hooks = hooks_of(type_info);
	if (move && hooks.move) {
		hooks.move(destination, value, 1);
	}
	else if (hooks.copy) {
		hooks.copy(destination, value, 1);
	}
	else {
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
	system_desc.entity = ecs_entity_init(_impl->c(), &entity_desc);

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

	return EntityId(ecs_system_init(_impl->c(), &system_desc));
}

void* Ecs::_create_query(QueryDesc&& desc) {
	ecs_query_desc_t query_desc {};
	if (!fill_query_desc(*this, query_desc, desc)) {
		fassert(false, "A query named a component the ECS does not know");
		return nullptr;
	}
	query_desc.cache_kind = EcsQueryCacheAuto;
	return ecs_query_init(_impl->c(), &query_desc);
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
	ecs_iter_t it = ecs_query_iter(_impl->c(), static_cast<ecs_query_t*>(query));
	while (ecs_query_next(&it)) {
		SystemIterator iterator(*this, &it);
		callback(iterator, ctx);
	}
}

// ---- EcsObjects ----

EntityId Ecs::import_object(StaticString class_name) {
	if (auto it = _objects.find(class_name); it != _objects.end()) {
		return it->second;
	}

	const ClassInfo* info = ClassDB::get_class_info(class_name);
	if (!info || !info->object_create_func || !ClassDB::has_parent(info->name, EcsObject::get_class_static())) {
		return {};
	}
	std::unique_ptr<EcsObject> object = ClassDB::create_object<EcsObject>(class_name);
	if (!object) {
		return {};
	}

	// A module entity scopes everything the object declares, the way flecs namespaces its own modules.
	ecs_world_t* world = _impl->c();
	const StaticString name = _intern(class_name.str());
	ecs_entity_desc_t desc {};
	desc.name = name.data();
	const ecs_entity_t scope = ecs_entity_init(world, &desc);
	ecs_add_id(world, scope, EcsModule);

	// Recorded before on_import, so an object importing another that imports it back stops here.
	const EntityId id { scope };
	_objects[name] = id;

	std::println("Importing ECS object {}", class_name);
	const ecs_entity_t previous_scope = ecs_set_scope(world, scope);
	object->on_import(*this);
	ecs_set_scope(world, previous_scope);

	_object_instances.push_back(std::move(object));
	return id;
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

void* SystemIterator::_field_ptr(std::int32_t term, std::size_t size) const {
	auto* it = static_cast<ecs_iter_t*>(_impl);
	if (!it || term < 0 || term >= it->field_count || !ecs_field_is_set(it, static_cast<std::int8_t>(term))) {
		return nullptr;
	}
	return ecs_field_w_size(it, size, static_cast<std::int8_t>(term));
}

// ---- ComponentHandle ----

ComponentHandle::ComponentHandle(Ecs& ecs, EntityId entity, StaticString component) :
		_ecs(&ecs), _entity(entity), _component(ecs.find_component(component)) {
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
	return _ecs && !_component.is_null() && _ecs->is_valid(_entity)
		&& ecs_has_id(_ecs->_impl->c(), _entity.raw(), _component.raw());
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
	return true;
}

} //namespace feather
