module;

#include <framework/export_defs.h>

export module feather.core:framework.object;

import std;
import :framework.static_string;
import :framework.reflection_utils;

export namespace feather {

struct ClassInfo;
class Object;

// Registered ClassInfo for a class name, or null when unknown or ClassDB isn't up yet.
FEATHER_API const ClassInfo* find_class_info(std::string_view name);

// Non-owning, type-erased reference to a reflected object. ptr always points at the object's most-derived type,
// the one info describes, so it can only be built from a typed reference.
class FEATHER_API ObjectRef {
	friend class Reflected;
	friend class Object;
	friend class Variant;
	friend class ClassDB;

	void* _ptr = nullptr;
	const ClassInfo* _info = nullptr;

	ObjectRef(void* ptr, const ClassInfo* info) : _ptr { ptr }, _info { info } {}

public:
	ObjectRef() = default;

	// A Reflected type reports its dynamic type itself. A value type can't be derived from, so its static type is
	// its dynamic type; _feather_self rules out an unreflected subclass passing as its reflected base.
	template <is_reflected_class_type T>
		requires(!std::is_const_v<T> && (std::is_base_of_v<Reflected, T> || std::is_same_v<typename T::_feather_self, T>))
	ObjectRef(T& object) {
		if constexpr (std::is_base_of_v<Reflected, T>) {
			*this = object._get_object_ref();
		}
		else {
			_ptr = &object;
			_info = find_class_info(T::get_class_static());
		}
	}

	void* get() const noexcept { return _ptr; }
	const ClassInfo* get_class_info() const noexcept { return _info; }
	explicit operator bool() const noexcept { return _ptr != nullptr; }
	bool operator==(const ObjectRef&) const = default;

	StaticString get_class_name() const;
	bool is_a(const ClassInfo* target) const;
	// Pointer to the target base subobject, or null when the object isn't one.
	void* cast_to(const ClassInfo* target) const;

	template <is_reflected_class_type T>
	T* cast() const {
		return static_cast<T*>(cast_to(find_class_info(std::remove_const_t<T>::get_class_static())));
	}
};

// Owns one heap-allocated reflected object and destroys it through its ClassInfo's lifecycle hooks. The address
// never changes, so moving an Object leaves every ObjectRef taken from it valid.
class FEATHER_API Object {
	ObjectRef _ref;

	explicit Object(ObjectRef ref);

	static void* _allocate(std::size_t size, std::size_t alignment);
	static void _deallocate(void* ptr, std::size_t size, std::size_t alignment);
	void* _release(const ClassInfo* target);

public:
	Object() = default;
	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;
	Object(Object&& other) noexcept : _ref { std::exchange(other._ref, {}) } {}
	Object& operator=(Object&& other) noexcept;
	~Object() { reset(); }

	template <is_reflected_class_type T, class... TArgs>
	static Object make(TArgs&&... args);

	// Default-constructs a registered class; empty when it can't be default-constructed.
	static Object create(const ClassInfo& info);
	static Object create(std::string_view class_name);

	// Copy-constructs through the lifecycle hooks; empty when the class can't be copied.
	Object clone() const;

	ObjectRef ref() const noexcept { return _ref; }
	explicit operator bool() const noexcept { return static_cast<bool>(_ref); }

	// Hands the object over to the caller, who frees it with delete. Null, and nothing released, when the object
	// isn't a T.
	template <is_reflected_class_type T>
	T* release() {
		return static_cast<T*>(_release(find_class_info(std::remove_const_t<T>::get_class_static())));
	}

	void reset();
};

template <is_reflected_class_type T, class... TArgs>
Object Object::make(TArgs&&... args) {
	void* ptr = _allocate(sizeof(T), alignof(T));
	T* object;
	try {
		object = ::new (ptr) T(std::forward<TArgs>(args)...);
	}
	catch (...) {
		_deallocate(ptr, sizeof(T), alignof(T));
		throw;
	}
	return Object { ObjectRef(*object) };
}

} // namespace feather
