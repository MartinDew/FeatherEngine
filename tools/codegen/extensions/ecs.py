"""
ecs.py — ECS modifiers: EcsObject, Component.

Unlike core_modifiers.py these aren't part of the base modifier vocabulary in
principle (a headless, non-ECS consumer of this generator wouldn't need them),
but FeatherEngine's own core/world/* uses them, so they're shipped and always
loaded the same way core_modifiers.py is. A project (or a future core
subsystem) that never applies EcsObject/Component is unaffected either way --
these hooks only ever run for a class that actually carries the modifier
(Component's emit_dir is the one exception; see its docstring).
"""

from modifier_api import DirEmission, Modifier


class EcsObjectModifier(Modifier):
    """FCLASS(EcsObject) on an EcsObject subclass. Generates the static
    _import_object(WorldSim*) hook that WorldSim::init discovers via
    ClassDB::get_children_names("EcsObject") + get_static_method(child,
    "_import_object") (core/main/world_sim.cpp)."""
    name = "EcsObject"
    targets = frozenset({"class"})
    value_type = False

    def gen_body_lines(self, cls, ctx):
        return [("protected", "static void _import_object(WorldSim* sim);")]

    def bind_members_lines(self, cls, ctx):
        return [f'ClassDB::bind_static_method(&{cls.name}::_import_object, "_import_object", AccessLevel::Public);']

    def register_cpp_includes(self, cls, ctx):
        # The definition below needs a complete WorldSim, which the class's own
        # header only forward-declares; keeping the include here preserves that.
        return ["main/world_sim.h"]

    def register_cpp_definitions(self, cls, ctx):
        return [
            f"void {cls.name}::_import_object(WorldSim* sim) {{",
            f"\tsim->get_ecs()->import_object<{cls.name}>();",
            "}",
        ]


class ComponentModifier(Modifier):
    """FSTRUCT(Component) (or FCLASS(novtable, Component)) marks a value type
    as an ECS component. Doesn't touch the class body or _bind_members at
    all -- ecs.component<T>() needs a live Ecs&, which neither the per-class
    .gen.h macro nor _bind_members()/ClassDB ever has access to. Instead,
    registration is aggregated per source directory into one
    register_<dir>_components(Ecs&) via emit_dir(), which the engine calls
    once it actually has a world (see register_core_components)."""
    name = "Component"
    targets = frozenset({"class"})
    value_type = True

    def emit_dir(self, classes, ctx):
        members = [c for c in classes if any(m.name == self.name for m, _ in c.resolved_modifiers)]
        func = f"register_{ctx.dir_name}_components"
        cpp_lines = [f"void {func}(Ecs& ecs) {{"]
        cpp_lines += [f"\tecs.component<{c.name}>();" for c in members]
        cpp_lines += ["}", ""]
        return DirEmission(
            header_includes=["world/ecs.h"],
            header_decls=[f"void {func}(Ecs& ecs);", ""],
            cpp_includes=["world/ecs.h"],
            cpp_lines=cpp_lines,
        )


MODIFIERS = [EcsObjectModifier(), ComponentModifier()]
