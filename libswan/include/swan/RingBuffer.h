#pragma once

#include <stdint.h>
#include <atomic>
#include <utility>

namespace Swan {

template<typename T, size_t Size>
class AtomicRingBuffer {
public:
	bool canWrite()
	{
		return w_ % Size != (r_ - 1) % Size;
	}

	bool canRead()
	{
		return r_ % Size != w_ % Size;
	}

	void write(const T &val)
	{
		auto idx = w_.fetch_add(1);

		buffer_[idx % Size] = val;
	}

	void write(T &&val)
	{
		auto idx = w_.fetch_add(1);

		buffer_[idx % Size] = std::move(val);
	}

	T &read()
	{
		auto idx = r_.fetch_add(1);

		return buffer_[idx % Size];
	}

private:
	T buffer_[Size];
	std::atomic<size_t> r_ = 0;
	std::atomic<size_t> w_ = 0;
};

}
