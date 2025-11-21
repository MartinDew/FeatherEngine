#pragma once

#include <vector>

namespace feather {

template <class T, class U>
void swap_remove(std::vector<T, U>& vec, size_t index) {
	if (index >= vec.size()) {
		return;
	}

	vec[index] = vec.back();
	vec.pop_back();
}

} //namespace feather
