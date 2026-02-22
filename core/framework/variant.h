#pragma once

#include "container_utils.h"
#include "variant_array.h"

#include "reflected.h"
#include "rendering/triangle_mesh.h"
#include <math/math_defs.h>

#include <expected>
#include <string>
#include <type_traits>
#include <variant>

namespace feather {

enum class VariantType : uint8_t {
	NIL = 0,
	BOOL,
	// Math
	INT,
	FLOAT,
	VECTOR3,
	VECTOR2,
	VERTEX,
	COLOR,
	// Misc
	STRING,
	ARRAY,
	// Objects
	OBJECT,
	// Invalid
	INVALID
};

// Helper to get VariantType enum from C++ type
template <class T>
consteval VariantType get_variant_type() {
	if constexpr (std::is_same_v<T, bool>) {
		return VariantType::BOOL;
	}
	else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
		return VariantType::INT;
	}
	else if constexpr (std::is_floating_point_v<T>) {
		return VariantType::FLOAT;
	}
	else if constexpr (std::is_same_v<Vector3, T>) {
		return VariantType::VECTOR3;
	}
	else if constexpr (std::is_same_v<Vector2, T>) {
		return VariantType::VECTOR2;
	}
	else if constexpr (std::is_same_v<Color, T>) {
		return VariantType::COLOR;
	}
	else if constexpr (std::is_same_v<Vertex, T>) {
		return VariantType::VERTEX;
	}
	else if constexpr (std::is_same_v<T, std::string>) {
		return VariantType::STRING;
	}
	else if constexpr (std::is_same_v<T, VariantArray> || std::is_array_v<T> || is_contiguous_container<T>) {
		return VariantType::ARRAY;
	}
	else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_pointer_t<T>>) {
		return VariantType::OBJECT;
	}
	// Void case
	else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_void_v<T>) {
		return VariantType::NIL;
	}
	// Default case
	else {
		return VariantType::INVALID;
	}
}

// Type trait to check if a type can be converted to Variant
template <typename T>
concept VariantCompatible = get_variant_type<T>() != VariantType::INVALID;

class ClassInfo;

class Variant {
	using InternalVariant = std::variant<std::monostate, bool, size_t, real_t, Vector2, Vector3, Vertex, Color,
			std::string, VariantArray, Reflected*>;

	InternalVariant _data;
	VariantType _type;
	ClassInfo* _object_info = nullptr;

	void set_class_info(StaticString class_name);

public:
	// Default constructor - NIL
	Variant() : _data(std::monostate {}), _type(VariantType::NIL) {}

	Variant(Reflected& ref);

	// String literal constructor
	Variant(const char* str) : _data(std::string(str)), _type(VariantType::STRING) {}

	// Generic constructor with concept constraint
	template <VariantCompatible T>
		requires(!std::is_reference_v<T>)
	Variant(T value) {
		constexpr VariantType type = get_variant_type<T>();
		_type = type;

		if constexpr (type == VariantType::BOOL) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::INT) {
			_data = static_cast<size_t>(value);
		}
		else if constexpr (type == VariantType::FLOAT) {
			_data = static_cast<real_t>(value);
		}
		else if constexpr (type == VariantType::VECTOR3) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::STRING) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::ARRAY) {
			if constexpr (std::is_same_v<T, VariantArray>)
				_data = std::move(value);
			else {
				_data = VariantArray { value.begin(), value.end() };
			}
		}
		else if constexpr (type == VariantType::OBJECT) {
			_data = static_cast<Reflected*>(value);
			set_class_info(value->get_class_name());
		}
		else if constexpr (type == VariantType::NIL) {
			_data = std::monostate {};
		}
		// default case if assignment is 1:1
		else {
			_data = std::move(value);
		}
	}

	// Copy and move constructors
	Variant(const Variant& other) = default;
	Variant(Variant&& other) noexcept = default;

	// Assignment operators
	Variant& operator=(const Variant& other) = default;
	Variant& operator=(Variant&& other) noexcept = default;

	// Type checking
	VariantType get_type() const { return _type; }
	bool is_type(VariantType type) const { return type == _type; }
	bool is_nil() const { return _type == VariantType::NIL; }

	// Type conversion with std::expected
	template <class T>
	std::expected<T, std::string> as() const {
		if (get_variant_type<T>() != _type)
			return std::unexpected("Variant type does not match requested type");

		try {
			if constexpr (std::is_same_v<T, bool>) {
				return std::get<bool>(_data);
			}
			else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
				return static_cast<T>(std::get<size_t>(_data));
			}
			else if constexpr (std::is_floating_point_v<T>) {
				return static_cast<T>(std::get<real_t>(_data));
			}
			else if constexpr (std::is_pointer_v<T> && is_reflected_class_type<std::remove_pointer_t<T>>) {
				return static_cast<T>(std::get<Reflected*>(_data));
			}
			else {
				// default case
				std::get<T>(_data);
			}
		} catch (const std::bad_variant_access&) {
			return std::unexpected("Bad variant access in Variant get");
		}
		std::unreachable();
	}

	// Equality operators
	bool operator==(const Variant& other) const;

	bool operator!=(const Variant& other) const { return !(*this == other); }

	// String conversion for debugging
	std::string to_string() const;

	// Direct access to internal variant (for advanced usage)
	const InternalVariant& internal() const { return _data; }

	// Object property access
	std::string get_name() const;
	Variant get(std::string_view key) const;
	void set(std::string_view key, const Variant& value);
	// Object method call
	Variant call(std::string_view method_name);
	Variant call(std::string_view method_name, auto&&... args);
};

} // namespace feather