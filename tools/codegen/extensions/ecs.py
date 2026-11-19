"""
ecs.py — Flecs ECS modifier: EcsModule.

Unlike core_modifiers.py this isn't part of the base modifier vocabulary in
principle (a headless, non-ECS consumer of this generator wouldn't need it),
but FeatherEngine's own world module uses it, so it's shipped and always
loaded the same way core_modifiers.py is. Its hooks only run for a class that
carries the modifier. Components need no modifier: every value type is
registered as a component from ClassDB at runtime (see WorldSim::init), except
one marked no_component.
"""

from modifier_api import Modifier


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


class NoComponentModifier(Modifier):
    """FSTRUCT(no_component): a value type that is never registered as an ECS component (see
    World::register_reflected_components). It only records the name in ClassInfo::modifiers."""
    name = "no_component"
    targets = frozenset({"class"})
    value_type = True


MODIFIERS = [EcsModuleModifier(), NoComponentModifier()]
