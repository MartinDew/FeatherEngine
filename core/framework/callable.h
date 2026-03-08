#pragma once

#include "variant.h"

#include "assert.h"

#include <cstdint>
#include <functional>
#include <span>
#include <tuple>
#include <type_traits>

namespace feather {

class Callable {
	std::function<Variant(std::span<Variant>)> _internal_func;

	// A function will likely not have more thant 255 params
	uint8_t _param_amount;

public:
	template <class TRet, class... TArgs>
		requires(VariantCompatible<TRet>) && (VariantCompatible<TArgs> && ...)
	Callable(std::function<TRet(TArgs...)> func)
			: _internal_func { [func](std::span<Variant> params) {
				size_t i = 0;
				std::tuple<TArgs...> converted_args = { params[i++].as<TArgs>().value()... };
				if constexpr (std::is_void_v<TRet>) {
					std::apply(func, converted_args);
					return Variant();
				}
				else {
					TRet result = std::apply(func, converted_args);
					return Variant(std::move(result));
				}
			} }
			, _param_amount { sizeof...(TArgs) } {}

	Callable(Callable&&) = default;
	Callable(const Callable&) = default;
	Callable& operator=(const Callable&) = default;
	Callable& operator=(Callable&&) = default;

	Variant call(std::span<Variant> params) {
		if (params.size() != _param_amount) {
			fassert(false,
					std::format("Callable called with incorrect number of parameters. Expected {} and got {}",
							_param_amount, params.size()));
		}
		return _internal_func(params);
	}

	Variant call(auto&&... args) {
		Variant params[] = { args... };
		return call(std::span<Variant>(params, sizeof...(args)));
	}
};

} //namespace feather