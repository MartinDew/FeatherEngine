"""
ecs.py — Flecs ECS modifiers: EcsModule.

A component needs no modifier: it derives from IComponent
(core/ecs/components/component_interface.h) and ClassDB records that parent,
which is what World watches to register it. Nothing is generated for a
component type any more.

Unlike core_modifiers.py this isn't part of the base modifier vocabulary in
principle (a headless, non-ECS consumer of this generator wouldn't need it),
but FeatherEngine's own core/ecs/* uses it, so it ships and is always loaded
the same way core_modifiers.py is. A project that never applies EcsModule is
unaffected: the hooks only run for a class carrying the modifier.
"""

from modifier_api import Modifier


class EcsModuleModifier(Modifier):
    """FCLASS(EcsModule) on an EcsModule subclass. Generates the static
    _import_module(World*) hook that World::import_modules() discovers via
    ClassDB::get_children_names("EcsModule") + get_static_method(child,
    "_import_module") (core/ecs/world.cpp) — previously hand-written per
    feature (see the old RenderingEcsModule::_load_module).

    Takes the World rather than the WorldSim that owns it: importing a module is
    the world's job, and a module header would otherwise need WorldSim complete
    just so ClassDB::bind_static_method could resolve VariantCompatible."""
    name = "EcsModule"
    targets = frozenset({"class"})
    value_type = False

    def gen_body_lines(self, cls, ctx):
        return [("protected", "static void _import_module(World* world);")]

    def bind_members_lines(self, cls, ctx):
        return [f'ClassDB::bind_static_method(&{cls.name}::_import_module, "_import_module", AccessLevel::Public);']

    def register_cpp_includes(self, cls, ctx):
        # Every module header already pulls in World through ecs_module.h, so
        # this is belt-and-braces rather than load-bearing -- but the generated
        # TU should not depend on that chain staying intact.
        return ["ecs/world.h"]

    def register_cpp_definitions(self, cls, ctx):
        return [
            f"void {cls.name}::_import_module(World* world) {{",
            f"\tworld->import_module<{cls.name}>();",
            "}",
        ]


MODIFIERS = [EcsModuleModifier()]
