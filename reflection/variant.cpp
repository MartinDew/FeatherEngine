module feather.reflection;

import std;

namespace feather {

namespace {

void* allocate_value(const ClassInfo& info) {
	if (info.alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		return ::operator new(info.size, std::align_val_t { info.alignment });
	}
	return ::operator new(info.size);
}

void free_value(void* ptr, const ClassInfo& info) noexcept {
	if (info.alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		::operator delete(ptr, info.size, std::align_val_t { info.alignment });
	}
	else {
		::operator delete(ptr, info.size);
	}
}

} // namespace

ValueBox::ValueBox(const ClassInfo& info, const void* src) {
	fassert(info.is_value_type && (src ? info.can_copy() : info.can_create()),
			src ? "Boxing a value type that can't be copied" : "Boxing a value type that can't be default-constructed");
	void* data = allocate_value(info);
	try {
		if (!src) {
			if (info.lifecycle.default_construct) {
				info.lifecycle.default_construct(data, 1);
			}
			else {
				std::memset(data, 0, info.size);
			}
		}
		else if (info.lifecycle.copy_construct) {
			info.lifecycle.copy_construct(data, src, 1);
		}
		else {
			std::memcpy(data, src, info.size);
		}
	} catch (...) {
		free_value(data, info);
		throw;
	}
	_info = &info;
	_data = data;
}

ValueBox ValueBox::ref(const ClassInfo& info, void* data) {
	fassert(info.is_value_type && data, "A value reference needs a value type instance");
	ValueBox box;
	box._info = &info;
	box._data = data;
	box._owned = false;
	return box;
}

ValueBox::ValueBox(const ValueBox& other)
		: ValueBox(!other._info ? ValueBox() : other._owned ? ValueBox(*other._info, other._data)
															: ref(*other._info, other._data)) {
}

ValueBox::ValueBox(ValueBox&& other) noexcept
		: _info { std::exchange(other._info, nullptr) }
		, _data { std::exchange(other._data, nullptr) }
		, _owned { std::exchange(other._owned, true) } {
}

ValueBox& ValueBox::operator=(const ValueBox& other) {
	if (this != &other) {
		*this = ValueBox(other);
	}
	return *this;
}

ValueBox& ValueBox::operator=(ValueBox&& other) noexcept {
	if (this != &other) {
		_destroy();
		_info = std::exchange(other._info, nullptr);
		_data = std::exchange(other._data, nullptr);
		_owned = std::exchange(other._owned, true);
	}
	return *this;
}

ValueBox::~ValueBox() {
	_destroy();
}

void ValueBox::_destroy() noexcept {
	if (_data && _owned) {
		if (_info->lifecycle.destruct) {
			_info->lifecycle.destruct(_data, 1);
		}
		free_value(_data, *_info);
	}
	_info = nullptr;
	_data = nullptr;
	_owned = true;
}

bool ValueBox::is_class(StaticString name) const noexcept {
	return _info && _info->name == name;
}

bool ValueBox::operator==(const ValueBox& other) const {
	if (_info != other._info) {
		return false;
	}
	if (!_info) {
		return true;
	}
	if (_info->lifecycle.equals) {
		return _info->lifecycle.equals(_data, other._data);
	}
	// With nothing to compare by, two distinct boxes are never assumed equal.
	bool compared = false;
	for (const ClassInfo::Property& property : _info->properties) {
		if (property.getter) {
			if (property.getter(_data) != property.getter(other._data)) {
				return false;
			}
			compared = true;
		}
	}
	return compared || _data == other._data;
}

const ClassInfo* Variant::_find_class_info(StaticString name) {
	return ClassDB::get_class_info(name);
}

Variant Variant::from_value(const ClassInfo& info, const void* src) {
	if (!info.is_value_type || !(src ? info.can_copy() : info.can_create())) {
		return {};
	}
	Variant result;
	result._data = ValueBox(info, src);
	result._type = VariantType::VALUE;
	return result;
}

Variant Variant::ref(const ClassInfo& info, void* data) {
	if (!info.is_value_type || !data) {
		return {};
	}
	Variant result;
	result._data = ValueBox::ref(info, data);
	result._type = VariantType::VALUE;
	return result;
}

Variant Variant::default_value(const ClassInfo& info) {
	return from_value(info, nullptr);
}

namespace {

template <class TVisit>
bool visit_builtin_type(VariantType type, TVisit&& visit) {
	switch (type) {
		case VariantType::VECTOR2: visit(std::type_identity<Vector2> {}); return true;
		case VariantType::VECTOR3: visit(std::type_identity<Vector3> {}); return true;
		case VariantType::VECTOR4: visit(std::type_identity<Vector4> {}); return true;
		case VariantType::QUATERNION: visit(std::type_identity<Quaternion> {}); return true;
		case VariantType::COLOR: visit(std::type_identity<Color> {}); return true;
		case VariantType::MATRIX: visit(std::type_identity<Matrix> {}); return true;
		case VariantType::TRANSFORM: visit(std::type_identity<Transform> {}); return true;
		case VariantType::VERTEX: visit(std::type_identity<Vertex> {}); return true;
		default: return false;
	}
}

} // namespace

Variant Variant::from_builtin(VariantType type, const void* src) {
	Variant result;
	if (src) {
		visit_builtin_type(type, [&]<class T>(std::type_identity<T>) { result = Variant(*static_cast<const T*>(src)); });
	}
	return result;
}

const void* Variant::builtin_data() const {
	const void* data = nullptr;
	visit_builtin_type(_type, [&]<class T>(std::type_identity<T>) { data = &std::get<T>(_data); });
	return data;
}

const ValueBox* Variant::get_value() const {
	return std::get_if<ValueBox>(&_data);
}

Reflected* Variant::_object() const {
	auto* object = std::get_if<Reflected*>(&_data);
	return object ? *object : nullptr;
}

bool Variant::operator==(const Variant& other) const {
	// Types must match
	if (_type != other._type) {
		return false;
	}

	// Use std::visit to compare the values
	return std::visit(
			[](const auto& lhs, const auto& rhs) -> bool {
				using LhsType = std::decay_t<decltype(lhs)>;
				using RhsType = std::decay_t<decltype(rhs)>;

				// If types don't match at compile time, return false
				if constexpr (!std::is_same_v<LhsType, RhsType>) {
					return false;
				}
				// monostate (NIL) is always equal
				else if constexpr (std::is_same_v<LhsType, std::monostate>) {
					return true;
				}
				// For all other types, use their operator==
				else {
					return lhs == rhs;
				}
			},
			_data,
			other._data
	);
}

std::string_view Variant::get_type_name(VariantType type) {
	switch (type) {
	case VariantType::NIL:
		return "Nil";
	case VariantType::BOOL:
		return "bool";
	case VariantType::INT:
		return "int";
	case VariantType::FLOAT:
		return "float";
	case VariantType::VECTOR3:
		return "Vector3";
	case VariantType::VECTOR2:
		return "Vector2";
	case VariantType::VERTEX:
		return "Vertex";
	case VariantType::COLOR:
		return "Color";
	case VariantType::VECTOR4:
		return "Vector4";
	case VariantType::QUATERNION:
		return "Quaternion";
	case VariantType::MATRIX:
		return "Matrix";
	case VariantType::TRANSFORM:
		return "Transform";
	case VariantType::RID:
		return "RID";
	case VariantType::STRING:
		return "String";
	case VariantType::ARRAY:
		return "Array";
	case VariantType::PATH:
		return "Path";
	case VariantType::OBJECT:
		return "Object";
	case VariantType::VALUE:
		return "Value";
	case VariantType::INVALID:
		break;
	}
	return "Invalid";
}

bool Variant::booleanize() const {
	return std::visit(
			[](const auto& value) -> bool {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, std::monostate>) {
					return false;
				}
				else if constexpr (std::is_same_v<T, bool>) {
					return value;
				}
				else if constexpr (std::is_arithmetic_v<T>) {
					return value != T {};
				}
				else if constexpr (
						std::is_same_v<T, std::string> || std::is_same_v<T, Path> || std::is_same_v<T, VariantArray>
				)
				{
					return !value.empty();
				}
				else if constexpr (std::is_same_v<T, Reflected*>) {
					return value != nullptr;
				}
				else if constexpr (std::is_same_v<T, ValueBox>) {
					return true;
				}
				else if constexpr (std::is_same_v<T, RID>) {
					return value != RID {};
				}
				else {
					// Math values: anything but the default (zero vector, identity quaternion/matrix/transform).
					return !(value == T {});
				}
			},
			_data
	);
}

std::string Variant::to_string() const {
	switch (_type) {
	case VariantType::NIL:
		return "nil";
	case VariantType::BOOL:
		return std::get<bool>(_data) ? "true" : "false";
	case VariantType::INT:
		return std::to_string(std::get<int64_t>(_data));
	case VariantType::FLOAT:
		return std::format("{:f}", std::get<real_t>(_data));
	case VariantType::STRING:
		return std::get<std::string>(_data);
	case VariantType::VECTOR2: {
		auto& v = std::get<Vector2>(_data);
		return std::format("({:f}, {:f})", v.x, v.y);
	}
	case VariantType::VECTOR3: {
		auto& v = std::get<Vector3>(_data);
		return std::format("({:f}, {:f}, {:f})", v.x, v.y, v.z);
	}
	case VariantType::COLOR: {
		auto& c = std::get<Color>(_data);
		return std::format("rgba({:f}, {:f}, {:f}, {:f})", c.r, c.g, c.b, c.a);
	}
	case VariantType::VECTOR4: {
		auto& v = std::get<Vector4>(_data);
		return std::format("({:f}, {:f}, {:f}, {:f})", v.x, v.y, v.z, v.w);
	}
	case VariantType::QUATERNION: {
		auto& q = std::get<Quaternion>(_data);
		return std::format("({:f}, {:f}, {:f}, {:f})", q.x, q.y, q.z, q.w);
	}
	case VariantType::MATRIX:
		return "[Matrix]";
	case VariantType::TRANSFORM:
		return "[Transform]";
	case VariantType::VERTEX:
		return "[Vertex]"; // expand if Vertex gains a meaningful string form
	case VariantType::PATH:
		return std::get<Path>(_data).string(); // assumes Path has to_string()
	case VariantType::RID:
		return "RID(" + std::to_string(std::get<RID>(_data).id) + ")"; // assumes RID has .id
	case VariantType::ARRAY:
		return "[Array]";
	case VariantType::OBJECT:
		return "[Object] : " + get_name();
	case VariantType::VALUE: {
		const ValueBox& box = std::get<ValueBox>(_data);
		return box.get_class_info() ? std::format("[{}]", box.get_class_info()->name.str()) : "[Value]";
	}
	case VariantType::INVALID:
		return "[Invalid]";
	}
	return "[Unknown]";
}

std::string Variant::get_name() const {
	if (_type != VariantType::OBJECT) {
		return to_string();
	}
	Reflected* object = _object();
	return object ? std::string(object->get_class_name()) : "[Object: null]";
}

namespace {

template <class T>
concept has_builtin_ftype = requires { FType<T>::properties; };

} // namespace

bool Variant::_is_reflectable() const {
	return _type == VariantType::OBJECT || _type == VariantType::VALUE || _has_builtin_properties();
}

bool Variant::_has_builtin_properties() const {
	return std::visit([](const auto& value) { return has_builtin_ftype<std::decay_t<decltype(value)>>; }, _data);
}

Variant Variant::_get_builtin(std::string_view key) const {
	return std::visit(
			[&](const auto& value) -> Variant {
				using T = std::decay_t<decltype(value)>;
				Variant result;
				if constexpr (has_builtin_ftype<T>) {
					std::apply(
							[&](const auto&... desc) {
								((desc.name == key ? (result = Variant(std::invoke(desc.getter, value)), true) : false) ||
								 ...);
							},
							FType<T>::properties
					);
				}
				return result;
			},
			_data
	);
}

void Variant::_set_builtin(std::string_view key, const Variant& new_value) {
	std::visit(
			[&](auto& value) {
				using T = std::decay_t<decltype(value)>;
				if constexpr (has_builtin_ftype<T>) {
					std::apply(
							[&](const auto&... desc) {
								auto assign = [&](const auto& property) {
									if (property.name != key) {
										return false;
									}
									constexpr auto setter = std::decay_t<decltype(property)>::setter;
									using Value = typename reflection_detail::accessor_value<setter, false>::type;
									if (auto converted = new_value.as<Value>()) {
										if constexpr (reflection_detail::is_data_member<setter>) {
											value.*setter = std::move(*converted);
										}
										else {
											(value.*setter)(std::move(*converted));
										}
									}
									return true;
								};
								(assign(desc) || ...);
							},
							FType<T>::properties
					);
				}
			},
			_data
	);
}

template <class TVisit>
bool Variant::_walk(TVisit&& visit) {
	if (_type == VariantType::OBJECT) {
		Reflected* object = _object();
		const ClassInfo* info = object ? object->get_class_info() : nullptr;
		return info && info->walk(object, std::forward<TVisit>(visit));
	}
	if (ValueBox* box = std::get_if<ValueBox>(&_data); box && box->get()) {
		return visit(*box->get_class_info(), box->get());
	}
	return false;
}

Variant Variant::_get_property(std::string_view key, bool enforce_public) const {
	if (_has_builtin_properties()) {
		return _get_builtin(key);
	}
	Variant result;
	// Getters take a mutable pointer but never write through it.
	const_cast<Variant&>(*this)._walk([&](const ClassInfo& info, void* object) {
		for (auto& property : info.properties) {
			if (property.name == key) {
				// Script-facing access: only public getters are reachable.
				if (property.getter && (!enforce_public || property.getter_access == AccessLevel::Public)) {
					result = property.getter(object);
				}
				return true;
			}
		}
		return false;
	});
	return result;
}

void Variant::_set_property(std::string_view key, const Variant& value, bool enforce_public) {
	if (_has_builtin_properties()) {
		_set_builtin(key, value);
		return;
	}
	_walk([&](const ClassInfo& info, void* object) {
		for (auto& property : info.properties) {
			if (property.name == key) {
				// Script-facing access: only public setters are writable.
				if (property.setter && (!enforce_public || property.setter_access == AccessLevel::Public)) {
					property.setter(object, value);
				}
				return true;
			}
		}
		return false;
	});
}

Variant Variant::get(std::string_view key) const {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	return _get_property(key, true);
}

Variant Variant::get_internal(std::string_view key) const {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	return _get_property(key, false);
}

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	_set_property(key, value, true);
}

void Variant::set_internal(std::string_view key, const Variant& value) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	_set_property(key, value, false);
}

Variant Variant::call(std::string_view method_name) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	Variant self;
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/true);
}

Variant Variant::call_internal(std::string_view method_name) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	Variant self;
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/false);
}

Variant Variant::_internal_call(std::string_view method_name, std::span<Variant> args, bool enforce_public) {
	fassert(!_has_builtin_properties(), "Built-in types have no methods");
	const ClassInfo* info = nullptr;
	if (Reflected* object = _object()) {
		info = object->get_class_info();
	}
	else if (const ValueBox* box = get_value()) {
		info = box->get_class_info();
	}
	for (; info; info = info->parent_info) {
		for (const ClassInfo::Method& method : info->methods) {
			if (method.name != method_name) {
				continue;
			}
			// Script-facing calls (enforce_public) only reach public methods.
			if (enforce_public && method.access != AccessLevel::Public) {
				return {};
			}
			if (method.is_static) {
				return method.callable.call(args.subspan(1));
			}
			if (_type == VariantType::OBJECT) {
				args[0] = Variant(_object());
				return method.callable.call(args);
			}
			// The box moves into args[0] and back, so the method works on this instance, which never changes address.
			struct Restore {
				Variant& owner;
				Variant& self;
				~Restore() { owner = std::move(self); }
			} restore { *this, args[0] };
			args[0] = std::move(*this);
			return method.callable.call(args);
		}
	}
	return {};
}

} //namespace feather
