#pragma once

// What a reflected class body names, and nothing more.
//
// The generated body only declares _bind_members() -- its definition lives in
// register_<dir>_types.gen.cpp -- so a reflected header needs no ClassDB, only
// the two names its friend declarations mention. Keeping it that way is what
// lets a type Variant itself stores (Vector3, Color) be reflected: pulling
// class_db.inl into every reflected header would need Variant complete while
// Variant is still being defined.
namespace feather {

class ClassDB;

template <class T>
void has_bind_method(const T& t);

} //namespace feather
