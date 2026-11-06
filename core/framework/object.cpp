module feather.core;

import std;

namespace feather {

StaticString ObjectRef::get_class_name() const {
	return _info ? _info->name : ""_ss;
}

bool ObjectRef::is_a(const ClassInfo* target) const {
	return cast_to(target) != nullptr;
}

void* ObjectRef::cast_to(const ClassInfo* target) const {
	if (!target) {
		return nullptr;
	}
	const ClassInfo* info = _info;
	void* object = _ptr;
	while (info && object) {
		if (info == target) {
			return object;
		}
		if (!info->to_parent) {
			return nullptr;
		}
		object = info->to_parent(object);
		info = ClassDB::get_class_info(info->parent);
	}
	return nullptr;
}

Object::Object(ObjectRef ref) : _ref { ref } {
	fassert(_ref._info, "Object created for a class that isn't registered");
	_ref._info->live_objects.fetch_add(1, std::memory_order_relaxed);
}

Object& Object::operator=(Object&& other) noexcept {
	if (this != &other) {
		reset();
		_ref = std::exchange(other._ref, {});
	}
	return *this;
}

// Allocates the way a new-expression would, so a released object can be freed with a plain delete.
void* Object::_allocate(std::size_t size, std::size_t alignment) {
	if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		return ::operator new(size, std::align_val_t { alignment });
	}
	return ::operator new(size);
}

void Object::_deallocate(void* ptr, std::size_t size, std::size_t alignment) {
	if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		::operator delete(ptr, size, std::align_val_t { alignment });
	}
	else {
		::operator delete(ptr, size);
	}
}

Object Object::create(const ClassInfo& info) {
	if (!info.can_create()) {
		return {};
	}
	void* ptr = _allocate(info.size, info.alignment);
	if (info.value_hooks.default_construct) {
		try {
			info.value_hooks.default_construct(ptr, 1);
		}
		catch (...) {
			_deallocate(ptr, info.size, info.alignment);
			throw;
		}
	}
	else {
		// Trivially default-constructible: value-initialization zeroes it.
		std::memset(ptr, 0, info.size);
	}
	return Object { ObjectRef(ptr, &info) };
}

Object Object::create(std::string_view class_name) {
	const ClassInfo* info = ClassDB::get_class_info(class_name);
	return info ? create(*info) : Object {};
}

Object Object::clone() const {
	const ClassInfo* info = _ref._info;
	if (!info || has_flag(info->value_hooks.illegal, ValueTypeIllegal::CopyConstruct)) {
		return {};
	}
	void* ptr = _allocate(info->size, info->alignment);
	if (info->value_hooks.copy_construct) {
		try {
			info->value_hooks.copy_construct(ptr, _ref._ptr, 1);
		}
		catch (...) {
			_deallocate(ptr, info->size, info->alignment);
			throw;
		}
	}
	else {
		std::memcpy(ptr, _ref._ptr, info->size);
	}
	return Object { ObjectRef(ptr, info) };
}

void* Object::_release(const ClassInfo* target) {
	void* object = _ref.cast_to(target);
	if (object) {
		_ref._info->live_objects.fetch_sub(1, std::memory_order_relaxed);
		_ref = {};
	}
	return object;
}

void Object::reset() {
	if (!_ref._ptr) {
		return;
	}
	const ClassInfo* info = _ref._info;
	if (info->value_hooks.destruct) {
		info->value_hooks.destruct(_ref._ptr, 1);
	}
	_deallocate(_ref._ptr, info->size, info->alignment);
	info->live_objects.fetch_sub(1, std::memory_order_relaxed);
	_ref = {};
}

} // namespace feather
