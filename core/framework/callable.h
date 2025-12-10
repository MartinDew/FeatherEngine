#pragma once

#include "variant.h"

#include "assert.h"

#include <cstdint>
#include <functional>
#include <span>

namespace feather {

class Callable {
	std::function<Variant(std::span<Variant>)> _internal_func;

	// A function will likely not have more thant 256 params
	uint8_t _param_amount;

public:
	template <class TRet, class... Args>
		requires(VariantCompatible<TRet>) && ((VariantCompatible<Args>) && ...)
	Callable(std::function<TRet(Args...)> func)
			: _internal_func { [func](std::span<Variant> params) {
				size_t i = 0;
				std::tuple<Args&...> converted_args = { params[i++].as<Args>... };
				return std::apply(func, converted_args);
			} }
			, _param_amount { sizeof...(Args) } {}

	Variant call(std::span<Variant> params) {
		if (params.size() != _param_amount) {
			fassert(false, "Callable called with incorrect number of parameters");
		}
		return _internal_func(params);
	}

	Variant call(auto&&... args) {
		Variant params[] = { args... };
		return call(std::span<Variant>(params, sizeof...(args)));
	}
};

} //namespace feather