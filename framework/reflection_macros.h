#pragma once

struct NO_PARENT {};

// Token-paste helpers (two levels so args like __LINE__/CURRENT_FILE_ID expand
// before being pasted).
#define FEATHER_JOIN_INNER(a, b) a##b
#define FEATHER_JOIN(a, b) FEATHER_JOIN_INNER(a, b)

// Marks a reflected class; the generator (tools/codegen/generate_reflection.py)
// recovers name/parent from the declaration itself. Optional modifiers:
// singleton, abstract, novtable (no vtable -- see FSTRUCT). Properties and
// methods are opt-in via [[get]]/[[set]]/[[name(...)]] / [[method]] attributes.
//
// Expands to a per-class body macro the generator writes into "<header>.gen.h";
// that include must be the LAST include in the header, since each generated
// header redefines CURRENT_FILE_ID and FCLASS needs it to name the right file.
//
// FSTRUCT is FCLASS with `novtable` implied -- for a plain-property type (e.g.
// an ECS component) with no polymorphism, factory, or Reflected base.
#ifdef FEATHER_REFLECTION_PARSER
// Generator parses with this defined so extraction never depends on
// possibly-stale generated output.
#define FCLASS(...)
#define FSTRUCT(...)
#else
#define FCLASS(...) FEATHER_JOIN(CURRENT_FILE_ID, FEATHER_JOIN(_, FEATHER_JOIN(__LINE__, _GEN_BODY)))()
#define FSTRUCT(...) FEATHER_JOIN(CURRENT_FILE_ID, FEATHER_JOIN(_, FEATHER_JOIN(__LINE__, _GEN_BODY)))()
#endif

// Marks a namespace-scope enum for runtime reflection (names <-> values, see ClassDB::get_enum). Prefix the
// declaration: `FENUM()` on its own line before `enum class Foo { A, B };`. Nothing is generated into the enum itself.
#define FENUM(...)
