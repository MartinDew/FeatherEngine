module;

#include <flecs.h>

module feather.world;

import std;

namespace feather {

namespace {

const LifecycleHooks& lifecycle_of(const ecs_type_info_t* type_info) {
	return *static_cast<const LifecycleHooks*>(type_info->hooks.binding_ctx);
}

void hook_ctor(void* ptr, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).default_construct(ptr, static_cast<std::size_t>(count));
}

void hook_dtor(void* ptr, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).destruct(ptr, static_cast<std::size_t>(count));
}

void hook_copy(void* dst, const void* src, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).copy(dst, src, static_cast<std::size_t>(count));
}

void hook_move(void* dst, void* src, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).move(dst, src, static_cast<std::size_t>(count));
}

void hook_copy_ctor(void* dst, const void* src, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).copy_construct(dst, src, static_cast<std::size_t>(count));
}

void hook_move_ctor(void* dst, void* src, std::int32_t count, const ecs_type_info_t* type_info) {
	lifecycle_of(type_info).move_construct(dst, src, static_cast<std::size_t>(count));
}

// A null hook that is not illegal is trivial, which is also what flecs assumes for a hook it is not given.
ecs_type_hooks_t to_flecs_hooks(const LifecycleHooks& lifecycle) {
	ecs_type_hooks_t hooks {};
	hooks.binding_ctx = const_cast<LifecycleHooks*>(&lifecycle);
	hooks.ctor = lifecycle.default_construct ? hook_ctor : nullptr;
	hooks.dtor = lifecycle.destruct ? hook_dtor : nullptr;
	hooks.copy = lifecycle.copy ? hook_copy : nullptr;
	hooks.move = lifecycle.move ? hook_move : nullptr;
	hooks.copy_ctor = lifecycle.copy_construct ? hook_copy_ctor : nullptr;
	hooks.move_ctor = lifecycle.move_construct ? hook_move_ctor : nullptr;

	const LifecycleIllegal illegal = lifecycle.illegal;
	if (has_flag(illegal, LifecycleIllegal::DefaultConstruct)) {
		hooks.flags |= ECS_TYPE_HOOK_CTOR_ILLEGAL;
	}
	if (has_flag(illegal, LifecycleIllegal::Destruct)) {
		hooks.flags |= ECS_TYPE_HOOK_DTOR_ILLEGAL;
	}
	if (has_flag(illegal, LifecycleIllegal::Copy)) {
		hooks.flags |= ECS_TYPE_HOOK_COPY_ILLEGAL;
	}
	if (has_flag(illegal, LifecycleIllegal::Move)) {
		hooks.flags |= ECS_TYPE_HOOK_MOVE_ILLEGAL;
	}
	if (has_flag(illegal, LifecycleIllegal::CopyConstruct)) {
		hooks.flags |= ECS_TYPE_HOOK_COPY_CTOR_ILLEGAL;
	}
	if (has_flag(illegal, LifecycleIllegal::MoveConstruct)) {
		hooks.flags |= ECS_TYPE_HOOK_MOVE_CTOR_ILLEGAL;
	}
	return hooks;
}

bool is_trivial(const LifecycleHooks& lifecycle) {
	return !lifecycle.default_construct && !lifecycle.destruct && !lifecycle.copy && !lifecycle.move &&
			!lifecycle.copy_construct && !lifecycle.move_construct && lifecycle.illegal == LifecycleIllegal::None;
}

const ClassInfo::Property* find_property(const ClassInfo& info, StaticString name) {
	for (const ClassInfo::Property& property : info.properties) {
		if (property.name == name) {
			return &property;
		}
	}
	return nullptr;
}

} //namespace

void World::_link_class_info(flecs::entity_t component, const ClassInfo& info) {
	flecs::entity(c_ptr(), component).set<ReflectedComponent>({ &info });
}

flecs::entity World::register_component(const ClassInfo& info) {
	if (const flecs::entity existing = get_component(info.name)) {
		return existing;
	}
	// Reflected objects have a parent and live on the heap; only parentless value types are stored in columns.
	if (!info.parent.str().empty() || info.is_abstract || info.is_singleton || !info.can_create()) {
		return {};
	}

	ecs_world_t* world = c_ptr();
	const std::string name(info.name.str());
	ecs_entity_desc_t entity_desc {};
	entity_desc.name = name.c_str();
	entity_desc.use_low_id = true;
	const ecs_entity_t scope = ecs_set_scope(world, 0);
	const ecs_entity_t entity = ecs_entity_init(world, &entity_desc);
	ecs_set_scope(world, scope);

	if (info.is_empty) {
		// flecs' C++ API registers an empty type as an entity with no EcsComponent, which is a tag.
		_link_class_info(entity, info);
		return flecs::entity(world, entity);
	}

	ecs_component_desc_t component_desc {};
	component_desc.entity = entity;
	component_desc.type.size = static_cast<ecs_size_t>(info.size);
	component_desc.type.alignment = static_cast<ecs_size_t>(info.alignment);
	if (!is_trivial(info.lifecycle)) {
		component_desc.type.hooks = to_flecs_hooks(info.lifecycle);
	}
	const ecs_entity_t component = ecs_component_init(world, &component_desc);
	fassert(component != 0, "flecs refused a component registered from reflection");

	_link_class_info(component, info);
	return flecs::entity(world, component);
}

flecs::entity World::register_component(StaticString class_name) {
	if (const flecs::entity existing = get_component(class_name)) {
		return existing;
	}
	const ClassInfo* info = ClassDB::get_class_info(class_name);
	return info ? register_component(*info) : flecs::entity {};
}

flecs::entity World::get_component(StaticString name) const {
	const std::string terminated(name.str());
	const ecs_entity_t found = ecs_lookup_child(c_ptr(), 0, terminated.c_str());
	if (!found) {
		return {};
	}
	// A tag registered from reflection has no EcsComponent, but carries its ClassInfo.
	const bool is_component =
			ecs_has_id(c_ptr(), found, ecs_id(EcsComponent)) || flecs::entity(c_ptr(), found).has<ReflectedComponent>();
	return is_component ? flecs::entity(c_ptr(), found) : flecs::entity {};
}

const ClassInfo* World::get_component_class_info(flecs::entity component) const {
	const ReflectedComponent* reflected = component ? component.try_get<ReflectedComponent>() : nullptr;
	return reflected ? reflected->info : nullptr;
}

Variant get_component_property(flecs::entity entity, flecs::entity component, StaticString property) {
	const World world(entity.world());
	const ClassInfo* info = world.get_component_class_info(component);
	const ClassInfo::Property* found = info ? find_property(*info, property) : nullptr;
	if (!found || !found->getter || found->getter_access != AccessLevel::Public) {
		return {};
	}
	const void* data = ecs_get_id(entity.world().c_ptr(), entity.id(), component.id());
	return data ? found->getter(const_cast<void*>(data)) : Variant {};
}

bool set_component_property(
		flecs::entity entity,
		flecs::entity component,
		StaticString property,
		const Variant& value
) {
	const World world(entity.world());
	const ClassInfo* info = world.get_component_class_info(component);
	const ClassInfo::Property* found = info ? find_property(*info, property) : nullptr;
	if (!found || !found->setter || found->setter_access != AccessLevel::Public) {
		return false;
	}
	void* data = ecs_get_mut_id(entity.world().c_ptr(), entity.id(), component.id());
	if (!data) {
		return false;
	}
	found->setter(data, value);
	entity.modified(component);
	return true;
}

Variant get_component_property(flecs::entity entity, StaticString component, StaticString property) {
	return get_component_property(entity, World(entity.world()).get_component(component), property);
}

bool set_component_property(flecs::entity entity, StaticString component, StaticString property, const Variant& value) {
	return set_component_property(entity, World(entity.world()).get_component(component), property, value);
}

} //namespace feather
