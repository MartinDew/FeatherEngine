#pragma once

#include "high_level_array.h"
#include "reflected.h"
#include <math/math_defs.h>

#include <expected>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

namespace feather {

enum class VariantType : uint8_t {
	NIL = 0,
	BOOL,
	INT,
	FLOAT,
	STRING,
	ARRAY,
	OBJECT,
	INVALID
};

// Type trait to check if a type can be converted to Variant
template <typename T>
concept VariantCompatible =
		(std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_floating_point_v<T> || std::is_same_v<T, bool> ||
		std::is_same_v<T, std::string> || std::is_same_v<T, HighLevelArray> || std::is_same_v<T, std::nullptr_t> ||
		(std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_pointer_t<T>> || std::is_void_v<T>);

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
	else if constexpr (std::is_same_v<T, std::string>) {
		return VariantType::STRING;
	}
	else if constexpr (std::is_same_v<T, HighLevelArray>) {
		return VariantType::ARRAY;
	}
	else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_pointer_t<T>>) {
		return VariantType::OBJECT;
	}
	else if constexpr (std::is_same_v<T, std::nullptr_t>) {
		return VariantType::NIL;
	}
	else {
		return VariantType::INVALID;
	}
}

class Variant {
	using InternalVariant = std::variant<std::monostate, bool, size_t, real_t, std::string, HighLevelArray, Reflected*>;

	InternalVariant _data;
	VariantType _type;
	void* _object_info = nullptr;

	// template <class T>
	// friend T* object_cast(Variant& var);

	void set_class_info(StaticString class_name);

public:
	// Default constructor - NIL
	Variant() : _data(std::monostate {}), _type(VariantType::NIL) {}

	// Generic constructor with concept constraint
	template <VariantCompatible T>
		requires(!std::is_reference_v<T>)
	Variant(T value) {
		if constexpr (std::is_same_v<T, bool>) {
			_data = value;
			_type = VariantType::BOOL;
		}
		else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			_data = static_cast<size_t>(value);
			_type = VariantType::INT;
		}
		else if constexpr (std::is_floating_point_v<T>) {
			_data = static_cast<real_t>(value);
			_type = VariantType::FLOAT;
		}
		else if constexpr (std::is_same_v<T, std::string>) {
			_data = value;
			_type = VariantType::STRING;
		}
		else if constexpr (std::is_same_v<T, HighLevelArray>) {
			_data = value;
			_type = VariantType::ARRAY;
		}
		else if constexpr (std::is_pointer_v<T> && is_reflected_class_type<std::remove_pointer_t<T>>) {
			_data = static_cast<Reflected*>(value);
			_type = VariantType::OBJECT;
			set_class_info(value->get_class_name());
		}
		else if constexpr (std::is_same_v<T, std::nullptr_t>) {
			_data = std::monostate {};
			_type = VariantType::NIL;
		}
	}

	Variant(Reflected& ref);

	// String literal constructor
	Variant(const char* str) : _data(std::string(str)), _type(VariantType::STRING) {}

	// Copy and move constructors
	Variant(const Variant& other) = default;
	Variant(Variant&& other) noexcept = default;

	// Assignment operators
	Variant& operator=(const Variant& other) = default;
	Variant& operator=(Variant&& other) noexcept = default;

	// Type checking
	VariantType get_type() const { return _type; }
	bool is_nil() const { return _type == VariantType::NIL; }
	bool is_bool() const { return _type == VariantType::BOOL; }
	bool is_int() const { return _type == VariantType::INT; }
	bool is_float() const { return _type == VariantType::FLOAT; }
	bool is_string() const { return _type == VariantType::STRING; }
	bool is_array() const { return _type == VariantType::ARRAY; }
	bool is_object() const { return _type == VariantType::OBJECT; }

	// Type conversion with std::expected
	template <class T>
	std::expected<T, std::string> as() const {
		try {
			if constexpr (std::is_same_v<T, bool>) {
				if (_type != VariantType::BOOL) {
					return std::unexpected("Variant type is not BOOL");
				}
				return std::get<bool>(_data);
			}
			else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
				if (_type != VariantType::INT) {
					return std::unexpected("Variant type is not INT");
				}
				return static_cast<T>(std::get<size_t>(_data));
			}
			else if constexpr (std::is_floating_point_v<T>) {
				if (_type != VariantType::FLOAT) {
					return std::unexpected("Variant type is not FLOAT");
				}
				return static_cast<T>(std::get<real_t>(_data));
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				if (_type != VariantType::STRING) {
					return std::unexpected("Variant type is not STRING");
				}
				return std::get<std::string>(_data);
			}
			else if constexpr (std::is_same_v<T, HighLevelArray>) {
				if (_type != VariantType::ARRAY) {
					return std::unexpected("Variant type is not ARRAY");
				}
				return std::get<HighLevelArray>(_data);
			}
			else if constexpr (std::is_pointer_v<T> && is_reflected_class_type<std::remove_pointer_t<T>>) {
				if (_type != VariantType::OBJECT) {
					return std::unexpected("Variant type is not OBJECT");
				}
				return static_cast<T>(std::get<Reflected*>(_data));
			}
			else {
				return std::unexpected("Unsupported type for Variant get");
			}
		} catch (const std::bad_variant_access&) {
			return std::unexpected("Bad variant access in Variant get");
		}
	}

	// Equality operators
	bool operator==(const Variant& other) const {
		if (_type != other._type)
			return false;
		return _data == other._data;
	}

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