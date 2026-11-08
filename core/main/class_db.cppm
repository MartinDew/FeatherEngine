module;

#include <framework/export_defs.h>
#include <framework/singleton_helpers.h>

export module feather.core:main.class_db;

import std.compat;
import :framework.static_string;
import :framework.callable;
import :framework.class_info;
import :framework.reflection_utils;
import :framework.variant;
import :framework.delegate;
import :framework.functions;

export namespace feather {

class FEATHER_API ClassDB {
	friend Variant;
	friend struct Main;
	FDECLARE_SINGLETON(ClassDB);

	ClassDB();
	~ClassDB();

public:
	using subclass_delegate_t = Delegate<const std::string_view>;

private:
	// Node-based, so a ClassInfo never moves once registered and ClassInfo* stays valid.
	std::unordered_map<StaticString, ClassInfo> _class_infos;
	std::unordered_map<StaticString, subclass_delegate_t> _subclass_delegates;

	static void _fire_subclass_delegates(std::string_view class_name);

	static std::vector<StaticString> _get_children_names_internal(const ClassInfo& object, bool exclusive = false);

	static ClassInfo* _get_class_info_internal(std::string_view name);

	template <is_reflected T>
	static void _register(ClassKind kind);

public:
	void print_db();

	// Empties the class registry. Must run before any extension DLL that
	// registered classes gets unloaded -- see Main::~Main().
	static void clear();

	static const ClassInfo* get_class_info(std::string_view name) { return _get_class_info_internal(name); };

	template <is_reflected T>
	static const ClassInfo* get_class_info() {
		return get_class_info(T::get_class_static());
	}

	static Callable get_static_method(const StaticString& class_name, std::string_view func_name);

	// Abstract, non-default-constructible and singleton classes are detected from T itself.
	template <is_reflected T>
	static void register_class() {
		_register<T>(ClassKind::Auto);
	}

	template <is_reflected T>
	static void register_abstract_class() {
		_register<T>(ClassKind::Abstract);
	}

	template <is_reflected T>
	static void register_singleton_class() {
		_register<T>(ClassKind::Singleton);
	}

	// New default-constructed instance owned by the caller, who frees it with destroy(); a Reflected one may instead
	// be deleted. Null when the class can't be created.
	static ObjectRef instantiate(std::string_view class_name);
	static void destroy(ObjectRef object);

	// Null when the class can't be created or isn't a T.
	template <is_reflected T>
	static std::unique_ptr<T> create_object(std::string_view class_name) {
		ObjectRef object = instantiate(class_name);
		T* typed = object.cast<T>();
		if (!typed) {
			destroy(object);
		}
		return std::unique_ptr<T> { typed };
	}

	static Delegate<std::string_view>::id_t on_subclass_registered(
			std::string_view base_class_name,
			const Delegate<std::string_view>::DelegateFuncType& callback
	);

	static void unregister_subclass_delegate(std::string_view base_class_name, Delegate<std::string_view>::id_t id);

	static std::vector<StaticString> get_children_names(std::string_view object_name, bool exclusive = false);

	static std::string get_children_names_string(StaticString object_name, bool exclusive = false);

	static bool has_parent(StaticString object_name, StaticString parent_name);
};

template <is_reflected T>
void ClassDB::_register(ClassKind kind) {
	if (kind == ClassKind::Auto) {
		if constexpr (std::is_abstract_v<T> || !std::is_default_constructible_v<T>) {
			kind = ClassKind::Abstract;
		}
		else if constexpr (is_singleton_v<T>) {
			kind = ClassKind::Singleton;
		}
	}
	const char* kind_name = kind == ClassKind::Abstract ? "abstract"
			: kind == ClassKind::Singleton				 ? "singleton"
														 : "concrete";
	std::println("Registering class '{}' as {} object", T::get_class_static(), kind_name);

	ClassDB& instance = *get();
	ClassInfo& info = instance._class_infos[T::get_class_static()];
	ClassInfoBuilder<T>::fill(info, kind);

	if (info.parent != "") {
		instance._class_infos[info.parent].children.emplace_back(&info);
	}

	_fire_subclass_delegates(T::get_class_static());
}

} //namespace feather
