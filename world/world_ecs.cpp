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

void free_lifecycle(void* lifecycle) {
	delete static_cast<LifecycleHooks*>(lifecycle);
}

// A null hook that is not illegal is trivial, which is also what flecs assumes for a hook it is not given. flecs owns
// a copy of the hooks, so the component never reads back into ClassDB.
ecs_type_hooks_t to_flecs_hooks(const LifecycleHooks& lifecycle) {
	ecs_type_hooks_t hooks {};
	hooks.binding_ctx = new LifecycleHooks(lifecycle);
	hooks.binding_ctx_free = free_lifecycle;
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

// The symbol flecs gives the C++ type: its compiler name with "::" turned into ".".
std::string get_symbol(const ClassInfo& info) {
	const std::string type_name(info.cpp_name.str());
	std::string symbol(type_name.size(), '\0');
	ecs_cpp_get_symbol_name(symbol.data(), type_name.c_str(), type_name.size());
	symbol.resize(std::strlen(symbol.c_str()));
	return symbol;
}

bool can_be_component(const ClassInfo& info) {
	return info.is_value_type && !info.has_modifier("no_component") && !info.is_abstract && !info.is_singleton &&
			info.can_create() && !info.cpp_name.str().empty();
}

} //namespace

void World::_link_class_info(flecs::entity_t component, const ClassInfo& info) {
	Entity(c_ptr(), component).set<ReflectedComponent>({ &info });
}

void World::_link_class_info(flecs::entity_t component, std::shared_ptr<const ClassInfo> info) {
	Entity(c_ptr(), component).set<ReflectedComponent>({ nullptr, std::move(info) });
}

flecs::entity_t World::_module_scope(StaticString module_name) {
	ecs_world_t* world = c_ptr();
	const std::string path =
			module_name.str().empty() ? std::string("feather") : std::format("feather.{}", module_name.str());
	const ecs_entity_t scope = ecs_set_scope(world, 0);
	const ecs_entity_t entity = ecs_new_from_path_w_sep(world, 0, path.c_str(), ".", nullptr);
	ecs_set_scope(world, scope);
	return entity;
}

void World::_place(flecs::entity_t entity, StaticString module_name, StaticString name) {
	ecs_world_t* world = c_ptr();
	const ecs_entity_t parent = _module_scope(module_name);
	const std::string wanted(name.str());
	if (ecs_get_parent(world, entity) != parent) {
		const ecs_entity_t taken = ecs_lookup_child(world, parent, wanted.c_str());
		fassert(!taken || taken == entity,
				std::format("feather.{}.{} already names another entity", module_name.str(), wanted));
		ecs_add_pair(world, entity, EcsChildOf, parent);
	}
	const char* current = ecs_get_name(world, entity);
	if (!current || wanted != current) {
		ecs_set_name(world, entity, wanted.c_str());
	}
}

Entity World::register_component(const ClassInfo& info) {
	if (!can_be_component(info)) {
		return {};
	}
	ecs_world_t* world = c_ptr();
	const std::string symbol = get_symbol(info);

	// Typed use in some binary may have registered the type already, possibly in a module's scope.
	if (const ecs_entity_t existing = ecs_lookup_symbol(world, symbol.c_str(), false, false)) {
		_place(existing, info.module_name, info.name);
		_link_class_info(existing, info);
		return Entity(world, existing);
	}

	const std::string name(info.name.str());
	const ecs_entity_t parent = _module_scope(info.module_name);
	fassert(!ecs_lookup_child(world, parent, name.c_str()),
			std::format("feather.{}.{} already names another entity", info.module_name.str(), name));

	ecs_entity_desc_t entity_desc {};
	entity_desc.parent = parent;
	entity_desc.name = name.c_str();
	entity_desc.symbol = symbol.c_str();
	entity_desc.use_low_id = true;
	const ecs_entity_t entity = ecs_entity_init(world, &entity_desc);

	// An empty type is a tag: a component of size 0, as flecs' C++ API registers it.
	ecs_component_desc_t component_desc {};
	component_desc.entity = entity;
	if (!info.is_empty) {
		component_desc.type.size = static_cast<ecs_size_t>(info.size);
		component_desc.type.alignment = static_cast<ecs_size_t>(info.alignment);
		if (!is_trivial(info.lifecycle)) {
			component_desc.type.hooks = to_flecs_hooks(info.lifecycle);
		}
	}
	const ecs_entity_t component = ecs_component_init(world, &component_desc);
	fassert(component != 0, "flecs refused a component registered from reflection");

	_link_class_info(component, info);
	return Entity(world, component);
}

Entity World::register_component(StaticString class_name) {
	const ClassInfo* info = ClassDB::get_class_info(class_name);
	return info ? register_component(*info) : Entity {};
}

void World::register_reflected_components() {
	_place(component<ReflectedComponent>(), StaticString(FMODULE_NAME), "ReflectedComponent"_ss);
	for (const StaticString& name : ClassDB::get_class_names()) {
		const ClassInfo* info = ClassDB::get_class_info(name);
		if (info && can_be_component(*info)) {
			register_component(*info);
		}
	}
}

EntityHandle World::create_entity(const std::string& name) {
	return name.empty() ? entity() : entity(name.c_str());
}

EntityHandle World::get_entity(const std::string& path) const {
	return lookup(path.c_str());
}

EntityHandle World::get_component(const std::string& class_name) const {
	const ClassInfo* info = ClassDB::get_class_info(StaticString(class_name));
	// Built-ins aren't in ClassDB: find the component linked to their ClassInfo by name.
	if (!info || info->cpp_name.str().empty()) {
		Entity builtin;
		each([&](Entity component, const ReflectedComponent& reflected) {
			if (!builtin && class_name == reflected.get_class_info()->name.str()) {
				builtin = component;
			}
		});
		return builtin;
	}
	const ecs_entity_t found = ecs_lookup_symbol(c_ptr(), get_symbol(*info).c_str(), false, false);
	return found ? Entity(c_ptr(), found) : Entity {};
}

const ClassInfo* World::get_component_class_info(Entity component) const {
	const ReflectedComponent* reflected = component ? component.try_get<ReflectedComponent>() : nullptr;
	return reflected ? reflected->get_class_info() : nullptr;
}

} //namespace feather
