"""
core_modifiers.py — the FCLASS/FSTRUCT modifiers every reflected type can use,
regardless of ECS or any other higher-level concern: singleton, abstract,
novtable, builtin. Always loaded (see build_registry in generate_reflection.py) — these
define the base modifier vocabulary itself, not an opt-in extension a project
chooses to pull in.

singleton/abstract used to be hardcoded directly into build_class()/
generate_gen_header()/generate_register_cpp(); porting them onto the Modifier
API (rather than keeping them as a special case alongside it) is what proves
the API is expressive enough for a project's own modifiers.
"""

from modifier_api import Modifier


class SingletonModifier(Modifier):
    """FCLASS(singleton). Reflection contract with FDECLARE_SINGLETON/
    FSINGLETON_INSTANCE (framework/singleton_helpers.h): adds the
    get()/instance boilerplate to the class body. The name reaches
    FType::modifiers, which ClassInfo::create_from_type reads to mark the
    class as not creatable through reflection (the engine constructs the
    single instance itself; reflection only ever resolves it via T::get())."""
    name = "singleton"
    targets = frozenset({"class"})
    value_type = False

    def gen_header_includes(self, cls, ctx):
        return ["framework/singleton_helpers.h"]

    def gen_body_lines(self, cls, ctx):
        return [("pre", f"FDECLARE_SINGLETON({cls.name});")]

class AbstractModifier(Modifier):
    """FCLASS(abstract). Usually redundant -- ClassInfo::create_from_type
    already auto-detects an abstract/non-default-constructible T -- this is
    the explicit "force it" escape hatch for a type that's abstract in spirit
    but not in the is_abstract_v<T>/default-constructible sense the
    auto-detection checks."""
    name = "abstract"
    targets = frozenset({"class"})
    value_type = False

class NovtableModifier(Modifier):
    """FCLASS(novtable) -- the `class`-spelling of what FSTRUCT is for
    `struct`. Purely a marker: ClassDesc.is_value_type is already computed
    directly from this token's presence in build_class(), before modifiers
    are even resolved through the registry (a purely syntactic scanner can't
    otherwise tell whether a base class spelled in another header eventually
    reaches Reflected, so this has to be an explicit, local signal -- see the
    ClassDesc.is_value_type docstring in generate_reflection.py). Registering
    it here only makes "novtable" a *known* token so FCLASS(novtable) doesn't
    trip the "unknown modifier" check; it has no codegen effect of its own."""
    name = "novtable"
    targets = frozenset({"class"})
    value_type = True


class BuiltinModifier(Modifier):
    """FSTRUCT(builtin) / FSTRUCT(builtin(Alias)) -- a type Variant stores natively (the math types). It gets an
    FType but no ClassDB registration and doesn't pass for a reflected value type. With an alias, the type that alias
    names in this build is described as the alias (Vector3f as "Vector3" when Vector3 = Vector3f)."""
    name = "builtin"
    targets = frozenset({"class"})
    value_type = True
    takes_args = True

    def ftype_name(self, cls, arg, ctx):
        if not arg:
            return None
        return f'std::is_same_v<{cls.name}, {arg}> ? "{arg}"_ss : "{cls.name}"_ss'

    def ftype_name_dependencies(self, cls, arg, ctx):
        return [arg] if arg else []

    def register_statements(self, cls, ctx):
        return []


MODIFIERS = [SingletonModifier(), AbstractModifier(), NovtableModifier(), BuiltinModifier()]
