module;

#include <framework/singleton_helpers.h>

export module feather.reflection:class_db;

import std.compat;
import feather.framework;
import :callable;
import :class_info;
import :enum_info;
import :reflection_utils;
import :reflected;
import :variant;

export namespace feather {

class FEATHER_REFLECTION_API ClassDB {
	friend Variant;
	FDECLARE_SINGLETON(ClassDB);

public:
	ClassDB();
	~ClassDB();

	using subclass_delegate_t = Delegate<const std::string_view>;

private:
	// Node-based, so a ClassInfo never moves once registered and ClassInfo* stays valid.
	std::unordered_map<StaticString, ClassInfo> _class_infos;
	std::unordered_map<StaticString, const EnumInfo*> _enum_infos;
	std::unordered_map<StaticString, subclass_delegate_t> _subclass_delegates;
	subclass_delegate_t _class_registered_delegate;

	static void _fire_subclass_delegates(const ClassInfo& info);

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(StaticString name);

	static Callable _find_static_method(StaticString class_name, std::string_view func_name, bool enforce_public);

	// Stores a built ClassInfo and links it to its parent; fires the registration delegates.
	static void _insert(ClassInfo info);

public:
	void print_db();

	// Empties the class registry. Must run before any extension DLL that
	// registered classes gets unloaded -- see Main::~Main().
	static void clear();

	inline static const ClassInfo* get_class_info(StaticString name) { return _get_class_info_internal(name); };

	template <has_ftype T>
	static const ClassInfo* get_class_info() {
		return get_class_info(FType<T>::name);
	}

	// Resolves `func_name` as Variant::call does: the nearest class up the hierarchy declaring it wins. Invalid when
	// that method isn't static, or isn't public (the _internal variant skips the access check).
	static Callable get_static_method(const StaticString& class_name, std::string_view func_name);
	// Like get static method but breaks access check
	static Callable get_static_method_unsafe(const StaticString& class_name, std::string_view func_name);

	// Register an enum before any class with a property of that type, so the property links to its EnumInfo.
	template <class E>
		requires std::is_enum_v<E>
	static void register_enum() {
		get()->_enum_infos[FEnum<E>::name] = &enum_info_v<E>;
	}

	static const EnumInfo* get_enum(StaticString name);

	template <class E>
		requires std::is_enum_v<E>
	static const EnumInfo* get_enum() {
		return get_enum(FEnum<E>::name);
	}

	// TODO: runtime registration (C API / scripts) needs a public register_class(ClassInfo), and
	// ClassInfo::Property getter/setter need user data, since only the templated _register<T> can insert today.

	// Abstract, not default-constructible and singleton classes are detected from T and its FType modifiers.
	template <is_reflected T>
	static void register_class() {
		_insert(ClassInfo::create_from_type<T>());
	}

	// New default-constructed object owned by the caller. Null when the class is unknown, is a value type or can't be
	// created.
	static Reflected* instantiate(StaticString class_name);

	// Null when the class can't be created or isn't a T.
	template <std::derived_from<Reflected> T>
		requires is_reflected<T>
	static std::unique_ptr<T> create_object(StaticString class_name) {
		std::unique_ptr<Reflected> object { instantiate(class_name) };
		T* typed = object_cast<T>(object.get());
		if (typed) {
			object.release();
		}
		return std::unique_ptr<T> { typed };
	}

	static Delegate<std::string_view>::id_t on_subclass_registered(
			std::string_view base_class_name,
			const Delegate<std::string_view>::DelegateFuncType& callback
	);

	static void unregister_subclass_delegate(std::string_view base_class_name, Delegate<std::string_view>::id_t id);

	// Fires with the class name after every registration, including classes from extensions loaded later.
	static Delegate<std::string_view>::id_t
	on_class_registered(const Delegate<std::string_view>::DelegateFuncType& callback);
	static void unregister_class_delegate(Delegate<std::string_view>::id_t id);

	static std::vector<StaticString> get_class_names();

	static std::vector<StaticString> get_children_names(StaticString object_name, bool exclusive = false);

	static std::string get_children_names_string(StaticString object_name, bool exclusive = false);

	static bool has_parent(StaticString object_name, StaticString parent_name);
};

} //namespace feather
