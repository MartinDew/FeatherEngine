"""
ecs.py — Flecs ECS modifiers: EcsModule, Component.

Unlike core_modifiers.py these aren't part of the base modifier vocabulary in
principle (a headless, non-ECS consumer of this generator wouldn't need them),
but FeatherEngine's own world module uses them, so they're shipped and always
loaded the same way core_modifiers.py is. A project (or a future core
subsystem) that never applies EcsModule/Component is unaffected either way --
these hooks only ever run for a class that actually carries the modifier
(Component's emit_dir is the one exception; see its docstring).
"""

from modifier_api import DirEmission, Modifier


class EcsModuleModifier(Modifier):
    """FCLASS(EcsModule) on an EcsModule subclass. Generates the static
    _import_module(WorldSim*) hook that WorldSim's constructor discovers via
    ClassDB::get_children_names("EcsModule") + get_static_method(child,
    "_import_module") (world/world_sim.cpp) — previously hand-written per
    feature (see the old RenderingWorldModule::_load_module)."""
    name = "EcsModule"
    targets = frozenset({"class"})
    value_type = False

    def gen_body_lines(self, cls, ctx):
        return [("protected", "static void _import_module(WorldSim* sim);")]

    def method_entries(self, cls, ctx):
        return [f'MethodDesc<&{cls.name}::_import_module> {{ "_import_module"_ss, AccessLevel::Public }}']

    def register_cpp_definitions(self, cls, ctx):
        return [
            f"void {cls.name}::_import_module(WorldSim* sim) {{",
            f"\tsim->get_world()->import<{cls.name}>();",
            "}",
        ]


class ComponentModifier(Modifier):
    """FSTRUCT(Component) (or FCLASS(novtable, Component)) marks a value type
    as a Flecs component. Doesn't touch the class body or its FType at
    all: registering a component needs a live world, which neither the
    per-class .gen.h macro nor ClassDB registration ever has. Instead,
    registration is aggregated per source directory into one
    register_<dir>_components(reg) via emit_dir(). It hands each component
    type to reg as reg.template operator()<T>(), so directories below world
    declare components without naming flecs; the engine passes a callback
    that registers them once it has a world (see register_core_components)."""
    name = "Component"
    targets = frozenset({"class"})
    value_type = True

    def emit_dir(self, classes, ctx):
        members = [c for c in classes if any(m.name == self.name for m, _ in c.resolved_modifiers)]
        func = f"register_{ctx.dir_name}_components"
        decls = ["template <class TRegister>", f"void {func}([[maybe_unused]] TRegister&& reg) {{"]
        decls += [f"\treg.template operator()<{c.name}>();" for c in members]
        decls += ["}", ""]
        return DirEmission(header_decls=decls)


MODIFIERS = [EcsModuleModifier(), ComponentModifier()]
