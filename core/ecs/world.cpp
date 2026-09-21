#include "world.h"

#include "component.h"
#include "components/component_interface.h"
#include "ecs_module.h"
#include "entity.h"
#include "flecs_backend.h"
#include "system_iterator.h"

#include <framework/assert.h>
#include <main/class_db.h>

#include <cstring>
#include <deque>
#include <format>

namespace feather {

namespace {

// flecs calls hooks per run of elements; ValueTypeOps is spelled the same way, so each one forwards directly rather
// than looping here.
void hook_ctor(void* ptr, int32_t count, const ecs_type_info_t* info) {
	const auto* ops = static_cast<const ValueTypeOps*>(info->hooks.binding_ctx);
	ops->default_construct(ptr, static_cast<size_t>(count));
}

void hook_dtor(void* ptr, int32_t count, const ecs_type_info_t* info) {
	const auto* ops = static_cast<const ValueTypeOps*>(info->hooks.binding_ctx);
	ops->destruct(ptr, static_cast<size_t>(count));
}

void hook_copy(void* dst, const void* src, int32_t count, const ecs_type_info_t* info) {
	const auto* ops = static_cast<const ValueTypeOps*>(info->hooks.binding_ctx);
	ops->copy(dst, src, static_cast<size_t>(count));
}

void hook_move(void* dst, void* src, int32_t count, const ecs_type_info_t* info) {
	const auto* ops = static_cast<const ValueTypeOps*>(info->hooks.binding_ctx);
	ops->move(dst, src, static_cast<size_t>(count));
}

// The ops a component's hooks read at runtime, kept alive for the process. A component cannot be withdrawn from a
// world that may already store it, so these are only ever added to.
const ValueTypeOps* keep(const ValueTypeOps& ops) {
	static auto* kept = new std::deque<ValueTypeOps>();
	kept->push_back(ops);
	return &kept->back();
}

size_t align_up(size_t offset, size_t alignment) {
	return (offset + alignment - 1) & ~(alignment - 1);
}

// The storage a runtime-described field occupies. Only trivially-copyable Variant types are allowed, because a
// described component is laid out and copied here rather than by a C++ type's own operators.
bool describe_field(VariantType type, size_t& out_size, size_t& out_alignment) {
	switch (type) {
		case VariantType::BOOL:
			out_size = sizeof(bool);
			out_alignment = alignof(bool);
			return true;
		case VariantType::INT:
			out_size = sizeof(int);
			out_alignment = alignof(int);
			return true;
		case VariantType::FLOAT:
			out_size = sizeof(real_t);
			out_alignment = alignof(real_t);
			return true;
		case VariantType::VECTOR2:
			out_size = sizeof(Vector2);
			out_alignment = alignof(Vector2);
			return true;
		case VariantType::VECTOR3:
			out_size = sizeof(Vector3);
			out_alignment = alignof(Vector3);
			return true;
		case VariantType::COLOR:
			out_size = sizeof(Color);
			out_alignment = alignof(Color);
			return true;
		default:
			return false;
	}
}

// Reads and writes go through memcpy rather than through a typed pointer: nothing guarantees the compiler's alignment
// assumptions for T hold at a computed offset inside storage flecs allocated.
template <typename T>
ClassInfo::Property make_memcpy_property(StaticString name, VariantType type, size_t offset) {
	return ClassInfo::Property {
		.name = name,
		.type = type,
		.getter =
				[offset](void* base) -> Variant {
					T value {};
					std::memcpy(&value, static_cast<std::byte*>(base) + offset, sizeof(T));
					return Variant(value);
				},
		.setter =
				[offset](void* base, Variant value) {
					if (auto converted = value.as<T>()) {
						const T stored = converted.value();
						std::memcpy(static_cast<std::byte*>(base) + offset, &stored, sizeof(T));
					}
				},
	};
}

ClassInfo::Property make_property(StaticString name, VariantType type, size_t offset) {
	switch (type) {
		case VariantType::BOOL:
			return make_memcpy_property<bool>(name, type, offset);
		case VariantType::INT:
			return make_memcpy_property<int>(name, type, offset);
		case VariantType::FLOAT:
			return make_memcpy_property<real_t>(name, type, offset);
		case VariantType::VECTOR2:
			return make_memcpy_property<Vector2>(name, type, offset);
		case VariantType::VECTOR3:
			return make_memcpy_property<Vector3>(name, type, offset);
		default:
			return make_memcpy_property<Color>(name, type, offset);
	}
}

} //namespace

// Holding the flecs world by pointer to an incomplete type is what lets world.h stay flecs-free.
struct World::Impl {
	flecs::world ecs;
};

StaticString World::_intern(std::string_view name) {
	// A class name reaches ClassInfo as a view, and flecs keeps the name it is given, so both need somewhere permanent
	// to point. Only ever added to, so a pointer handed out earlier stays good.
	static auto* names = new std::deque<std::string>();
	for (const std::string& existing : *names) {
		if (existing == name) {
			return StaticString(existing);
		}
	}
	names->emplace_back(name);
	return StaticString(names->back());
}

World::World() : _impl(std::make_unique<Impl>()) {
	_component_delegate = ClassDB::on_subclass_registered(
		IComponent::get_class_static(),
		[this](std::string_view class_name) { register_component(StaticString(class_name)); }
);
}

World::~World() {
	constexpr auto no_delegate = static_cast<ClassDB::subclass_delegate_t::id_t>(-1);
	if (_component_delegate != no_delegate) {
		ClassDB::unregister_subclass_delegate(IComponent::get_class_static(), _component_delegate);
	}
	if (_module_delegate != no_delegate) {
		ClassDB::unregister_subclass_delegate(EcsModule::get_class_static(), _module_delegate);
	}
}

// ---- Simulation ------------------------------------------------------------

bool World::progress(double delta) {
	return _impl->ecs.progress(static_cast<float>(delta));
}

EntityId World::create_timer(double interval) {
	return EntityId(_impl->ecs.timer().interval(static_cast<float>(interval)).raw_id());
}

float World::delta_time() const {
	return static_cast<float>(_impl->ecs.delta_time());
}

void World::enable_rest_api() {
	_impl->ecs.set<flecs::Rest>({});
}

// ---- Entities --------------------------------------------------------------

Entity World::create_entity(const std::string& name) {
	const flecs::entity created = name.empty() ? _impl->ecs.entity() : _impl->ecs.entity(name.c_str());
	return { *this, EntityId(created.raw_id()) };
}

Entity World::create_entity(const Entity& parent, const std::string& name) {
	Entity created = create_entity(name);
	return created.child_of(parent);
}

Entity World::entity(EntityId id) {
	return { *this, id };
}

Entity World::prefab(const std::string& name) {
	return { *this, EntityId(_impl->ecs.prefab(name.c_str()).raw_id()) };
}

Entity World::lookup(const std::string& name) {
	return { *this, EntityId(_impl->ecs.lookup(name.c_str()).raw_id()) };
}

void World::destroy_entity(EntityId id) const {
	_impl->ecs.entity(id.raw()).destruct();
}

bool World::is_valid(EntityId id) const {
	return !id.is_null() && _impl->ecs.entity(id.raw()).is_valid();
}

std::string World::get_entity_name(EntityId id) const {
	const char* name = _impl->ecs.entity(id.raw()).name();
	return name ? std::string(name) : std::string {};
}

void World::set_entity_name(EntityId id, const std::string& name) const {
	_impl->ecs.entity(id.raw()).set_name(name.c_str());
}

EntityId World::get_parent(EntityId id) const {
	return EntityId(_impl->ecs.entity(id.raw()).parent().raw_id());
}

void World::set_parent(EntityId child, EntityId parent) const {
	_impl->ecs.entity(child.raw()).child_of(_impl->ecs.entity(parent.raw()));
}

void World::set_prefab(EntityId entity, EntityId prefab) const {
	_impl->ecs.entity(entity.raw()).is_a(_impl->ecs.entity(prefab.raw()));
}

bool World::is_instance_of(EntityId entity, EntityId prefab) const {
	return _impl->ecs.entity(entity.raw()).has(flecs::IsA, _impl->ecs.entity(prefab.raw()));
}

// ---- Component types -------------------------------------------------------

EntityId World::register_component(StaticString class_name) {
	if (auto it = _components.find(class_name); it != _components.end()) {
		return it->second;
	}

	// IComponent itself is the empty base every component shares. It marks the family rather than describing storage,
	// so it is never registered.
	if (class_name == IComponent::get_class_static()) {
		return {};
	}

	const ClassInfo* info = ClassDB::get_class_info(class_name);
	if (!info || !info->is_value_type) {
		// An IComponent subclass is always a value type; anything else naming itself one is a mistake, and registering
		// it would give the world storage it cannot construct.
		return {};
	}

	return _register_component_raw(info->name, info->value_ops);
}

EntityId World::_register_component_raw(StaticString name, const ValueTypeOps& ops) {
	if (auto it = _components.find(name); it != _components.end()) {
		return it->second;
	}

	ecs_entity_desc_t entity_desc {};
	entity_desc.name = name.data();
	const ecs_entity_t entity = ecs_entity_init(_impl->ecs.c_ptr(), &entity_desc);
	if (!entity) {
		return {};
	}

	if (ops.size == 0) {
		// A zero-size component is a tag: it has no storage to get or set, so it stays a plain entity and is never
		// handed to ecs_component_init. Registered as one deliberately, not by accident.
		const EntityId tag { entity };
		_components[name] = tag;
		return tag;
	}

	ecs_component_desc_t desc {};
	desc.entity = entity;
	desc.type.size = static_cast<ecs_size_t>(ops.size);
	desc.type.alignment = static_cast<ecs_size_t>(ops.alignment);
	desc.type.name = name.data();

	// The ops are always kept, even for a fully trivial type: _set_component_raw reads them back out of binding_ctx to
	// decide between the copy hook and a memcpy.
	const ValueTypeOps* kept_ops = keep(ops);
	desc.type.hooks.binding_ctx = const_cast<ValueTypeOps*>(kept_ops);

	// Only the hooks the type actually needs; a null one tells flecs the operation is trivial and whole runs can be
	// moved with memcpy. The ctor is the exception: flecs zero-initializes on add when it is left null.
	desc.type.hooks.ctor = kept_ops->default_construct ? hook_ctor : flecs_default_ctor;
	desc.type.hooks.dtor = kept_ops->destruct ? hook_dtor : nullptr;
	desc.type.hooks.copy = kept_ops->copy ? hook_copy : nullptr;
	desc.type.hooks.move = kept_ops->move ? hook_move : nullptr;

	const ecs_entity_t component = ecs_component_init(_impl->ecs.c_ptr(), &desc);
	if (!component) {
		return {};
	}

	const EntityId id { component };
	_components[name] = id;
	return id;
}

EntityId World::find_component(StaticString class_name) const {
	auto it = _components.find(class_name);
	return it == _components.end() ? EntityId {} : it->second;
}

EntityId World::register_component(const std::string& name, std::span<const FieldDesc> fields, std::string* error) {
	auto fail = [&error](std::string message) -> EntityId {
		if (error) {
			*error = std::move(message);
		}
		return {};
	};

	if (name.empty()) {
		return fail("a component needs a name");
	}
	if (fields.empty()) {
		return fail(std::format("component '{}' has no fields", name));
	}
	if (_impl->ecs.lookup(name.c_str()).is_valid()) {
		return fail(std::format("'{}' already exists in the world", name));
	}

	// Laid out here, C-struct style: each field aligned to its own requirement, the whole padded to the widest.
	std::vector<ClassInfo::Property> properties;
	properties.reserve(fields.size());

	size_t offset = 0;
	size_t max_alignment = 1;
	for (const FieldDesc& field : fields) {
		size_t field_size = 0;
		size_t field_alignment = 1;
		if (!describe_field(field.type, field_size, field_alignment)) {
			return fail(std::format("field '{}' of component '{}' has a type the ECS cannot store", field.name, name));
		}
		if (field.name.empty()) {
			return fail(std::format("a field of component '{}' has no name", name));
		}

		offset = align_up(offset, field_alignment);
		max_alignment = std::max(max_alignment, field_alignment);
		properties.push_back(make_property(_intern(field.name), field.type, offset));
		offset += field_size;
	}

	ValueTypeOps ops;
	ops.size = align_up(offset, max_alignment);
	ops.alignment = max_alignment;
	// Every allowed field type is trivially copyable, so all four hooks stay null and flecs moves whole runs at once.

	const StaticString interned = _intern(name);

	// ClassDB first: it is the half that can refuse, and a refused registration must not leave a component behind
	// that nothing can read.
	if (!ClassDB::register_scripted_value_class(interned, std::move(properties), ops)) {
		return fail(std::format("'{}' is already a registered class", name));
	}

	// A scripted class registers parentless, so it never reached the IComponent delegate above -- registering it with
	// the world is this call's job, not the delegate's.
	const EntityId component = _register_component_raw(interned, ops);
	if (component.is_null()) {
		return fail(std::format("the world rejected the layout for component '{}'", name));
	}
	return component;
}

// ---- Components on entities ------------------------------------------------

bool World::add_component(EntityId entity, StaticString class_name) {
	const EntityId component = register_component(class_name);
	if (component.is_null()) {
		return false;
	}
	ecs_add_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
	return true;
}

bool World::has_component(EntityId entity, StaticString class_name) const {
	const EntityId component = find_component(class_name);
	return !component.is_null() && ecs_has_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
}

void World::remove_component(EntityId entity, StaticString class_name) {
	if (const EntityId component = find_component(class_name); !component.is_null()) {
		ecs_remove_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
	}
}

void World::remove_all(StaticString class_name) {
	if (const EntityId component = find_component(class_name); !component.is_null()) {
		ecs_remove_all(_impl->ecs.c_ptr(), component.raw());
	}
}

const void* World::component_data(EntityId entity, StaticString class_name) const {
	const EntityId component = find_component(class_name);
	return component.is_null() ? nullptr : ecs_get_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
}

void* World::mutable_component_data(EntityId entity, StaticString class_name) {
	const EntityId component = register_component(class_name);
	if (component.is_null()) {
		return nullptr;
	}

	const ecs_type_info_t* type_info = ecs_get_type_info(_impl->ecs.c_ptr(), component.raw());
	if (!type_info || type_info->size <= 0) {
		// A tag has no storage to hand back.
		return nullptr;
	}

	void* data = ecs_ensure_id(
			_impl->ecs.c_ptr(), entity.raw(), component.raw(), static_cast<size_t>(type_info->size)
	);
	if (data) {
		ecs_modified_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
	}
	return data;
}

bool World::_set_component_raw(EntityId entity, StaticString class_name, const void* value, size_t size) {
	const EntityId component = register_component(class_name);
	if (component.is_null()) {
		return false;
	}

	const ecs_type_info_t* type_info = ecs_get_type_info(_impl->ecs.c_ptr(), component.raw());
	if (!type_info || type_info->size <= 0) {
		// A tag carries no value; adding it is the whole operation.
		ecs_add_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
		return true;
	}
	if (static_cast<size_t>(type_info->size) != size) {
		return false;
	}

	void* destination = ecs_ensure_id(_impl->ecs.c_ptr(), entity.raw(), component.raw(), size);
	if (!destination) {
		return false;
	}

	// ecs_ensure_id hands back storage that has already been constructed, so this is an assignment: a type with a
	// copy hook must go through it rather than have its bytes overwritten.
	const auto* ops = static_cast<const ValueTypeOps*>(type_info->hooks.binding_ctx);
	if (ops && ops->copy) {
		ops->copy(destination, value, 1);
	}
	else {
		std::memcpy(destination, value, size);
	}

	ecs_modified_id(_impl->ecs.c_ptr(), entity.raw(), component.raw());
	return true;
}

// ---- Modules ---------------------------------------------------------------

EntityId World::_begin_module(StaticString class_name, EntityId& out_module) {
	// A module is an entity everything it declares is scoped under, which is how flecs namespaces a module's
	// components and systems.
	flecs::entity module_entity = _impl->ecs.entity(class_name.data());
	module_entity.add(flecs::Module);
	out_module = EntityId(module_entity.raw_id());
	return EntityId(ecs_set_scope(_impl->ecs.c_ptr(), out_module.raw()));
}

void World::_end_module(EntityId previous_scope) {
	ecs_set_scope(_impl->ecs.c_ptr(), previous_scope.raw());
}

bool World::is_module_imported(StaticString class_name) const {
	return _modules.contains(class_name);
}

void World::_import_module_by_name(StaticString class_name) {
	if (is_module_imported(class_name)) {
		return;
	}
	// The hook codegen emits for every EcsModule subclass; it calls back into import_module<T>, which is where the
	// module scope is opened and the subclass constructed.
	ClassDB::get_static_method(class_name, "_import_module").call(this);
}

void World::import_modules() {
	if (_module_delegate != static_cast<Delegate<std::string_view>::id_t>(-1)) {
		return;
	}

	// Subscribed before the sweep, so a module registering during another module's import is not missed.
	_module_delegate = ClassDB::on_subclass_registered(
			EcsModule::get_class_static(),
			[this](std::string_view class_name) { _import_module_by_name(StaticString(class_name)); }
	);

	for (StaticString name : ClassDB::get_children_names(EcsModule::get_class_static())) {
		_import_module_by_name(name);
	}
}

// ---- Systems and queries ---------------------------------------------------

namespace {

// Everything a registered system needs to survive for as long as the system does: the Feather-side callback, the
// user's callable, and how to destroy it. flecs owns this through callback_ctx and frees it via system_ctx_free.
struct SystemContext {
	World* world = nullptr;
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

// The single point where a flecs iteration becomes a Feather one.
void system_trampoline(ecs_iter_t* it) {
	auto* context = static_cast<SystemContext*>(it->callback_ctx);
	SystemIterator iterator(*context->world, it);
	context->callback(iterator, context->user_ctx);
}

// Takes its context from run_ctx rather than callback_ctx, and clears EcsIterIsValid instead of calling ecs_iter_fini
// (which would leave the pipeline wedged on the next system) -- matching flecs' own C++ run path.
void system_run_trampoline(ecs_iter_t* it) {
	auto* context = static_cast<SystemContext*>(it->run_ctx);
	it->flags &= ~EcsIterIsValid;
	SystemIterator iterator(*context->world, it);
	context->callback(iterator, context->user_ctx);
}

// Fills a flecs query description from a Feather one, resolving every term's component by name. Returns false when a
// term names something the world has no component for.
bool fill_query_desc(World& world, ecs_query_desc_t& out, const QueryDesc& desc) {
	if (desc.terms.size() > FLECS_TERM_COUNT_MAX) {
		return false;
	}

	for (size_t i = 0; i < desc.terms.size(); ++i) {
		const Term& term = desc.terms[i];
		// Registering rather than just looking up: a system can name a component before anything has instantiated one.
		const EntityId component = world.register_component(term.component);
		if (component.is_null()) {
			return false;
		}

		ecs_term_t& out_term = out.terms[i];
		out_term.id = component.raw();
		out_term.inout = to_flecs_inout(term.access);
		out_term.oper = term.optional ? EcsOptional : EcsAnd;
		out_term.src.id = to_flecs_traverse_flags(term.traverse);
		out_term.trav = to_flecs_traversal_relationship(term.traverse);
	}
	return true;
}

} //namespace

EntityId World::_register_system(SystemDesc&& desc) {
	ecs_system_desc_t system_desc {};
	if (!fill_query_desc(*this, system_desc.query, desc.query)) {
		fassert(false, "a system named a component the world does not know");
		return {};
	}

	ecs_entity_desc_t entity_desc {};
	entity_desc.name = desc.query.name.empty() ? nullptr : desc.query.name.c_str();
	system_desc.entity = ecs_entity_init(_impl->ecs.c_ptr(), &entity_desc);

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

	system_desc.phase = to_flecs_phase(desc.phase);
	system_desc.multi_threaded = desc.multi_threaded;
	system_desc.tick_source = desc.tick_source.raw();

	return EntityId(ecs_system_init(_impl->ecs.c_ptr(), &system_desc));
}

void* World::_create_query(QueryDesc&& desc) {
	ecs_query_desc_t query_desc {};
	if (!fill_query_desc(*this, query_desc, desc)) {
		fassert(false, "a query named a component the world does not know");
		return nullptr;
	}
	return ecs_query_init(_impl->ecs.c_ptr(), &query_desc);
}

void World::_destroy_query(void* query) const {
	if (query) {
		ecs_query_fini(static_cast<ecs_query_t*>(query));
	}
}

void World::_query_each(void* query, SystemCallback callback, void* ctx) {
	if (!query || !callback) {
		return;
	}
	ecs_iter_t it = ecs_query_iter(_impl->ecs.c_ptr(), static_cast<ecs_query_t*>(query));
	while (ecs_query_next(&it)) {
		SystemIterator iterator(*this, &it);
		callback(iterator, ctx);
	}
}

// ---- Reflected surface -----------------------------------------------------

bool World::has_component_type(std::string class_name) const {
	return !find_component(StaticString(class_name)).is_null();
}

bool World::register_component_type_by_name(std::string class_name) {
	// The name has to outlive the call: it arrives as a std::string from a script, and the world keeps a view of it.
	return !register_component(_intern(class_name)).is_null();
}

bool World::has_module(std::string class_name) const {
	return is_module_imported(StaticString(class_name));
}

int World::get_component_type_count() const {
	return static_cast<int>(_components.size());
}

} //namespace feather
