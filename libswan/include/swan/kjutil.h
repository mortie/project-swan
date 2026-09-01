#pragma once

#include <kj/array.h>
#include <cstddef>

namespace Swan {

template<typename T>
kj::Array<T> kjZeroedArray(size_t size)
{
	auto arr = kj::heapArray<T>(size);
	memset(&arr.front(), 0, arr.asBytes().size());
	return arr;
}

}
