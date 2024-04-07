#pragma once

#include <stddef.h>
#include <assert.h>
#include <memory>

namespace SwanCommon {

template<typename T, typename Size = size_t>
struct LruCacheEntry {
	T value;
	Size prev;
	Size next;
};

template<typename T, typename Size = size_t>
class LruCache {
public:
	LruCache()
	{}

	LruCache(Size size): data_(std::make_unique<Entry[]>(size))
	{
		initialize(size);
	}

	void reset(Size size)
	{
		data_ = std::make_unique<Entry[]>(size);
		initialize(size);
	}

	void bump(Size idx)
	{
		Entry &ent = data_[idx];

		if (ent.prev != NULL_IDX) {
			data_[ent.prev].next = ent.next;
		}

		if (ent.next != NULL_IDX) {
			data_[ent.next].prev = ent.prev;
		}

		if (idx == last_) {
			last_ = ent.prev;
		}

		data_[first_].prev = idx;
		ent.next = first_;
		ent.prev = NULL_IDX;
		first_ = idx;
	}

	Size next()
	{
		Size idx = nextFree();

		if (idx == NULL_IDX) {
			idx = nextUsed();
		}

		return idx;
	}

	Size nextFree()
	{
		if (firstFree_ == NULL_IDX) {
			return NULL_IDX;
		}

		Size idx = firstFree_;
		Entry &ent = data_[idx];

		firstFree_ = ent.next;

		if (first_ != NULL_IDX) {
			data_[first_].prev = idx;
		}

		ent.next = first_;
		ent.prev = NULL_IDX;
		first_ = idx;

		if (last_ == NULL_IDX) {
			last_ = idx;
		}

		return idx;
	}

	Size nextUsed()
	{
		if (first_ == NULL_IDX) {
			return NULL_IDX;
		}

		Size idx = last_;
		Entry &ent = data_[idx];

		if (ent.prev != NULL_IDX) {
			data_[ent.prev].next = NULL_IDX;
			ent.prev = NULL_IDX;
		}

		data_[first_].prev = idx;
		ent.next = first_;
		ent.prev = NULL_IDX;
		first_ = idx;

		return idx;
	}

	T &operator[](Size idx)
	{
		return data_[idx].value;
	}

	Size null()
	{
		return NULL_IDX;
	}

private:
	using Entry = LruCacheEntry<T, Size>;

	static constexpr Size NULL_IDX = (Size) - 1;

	void initialize(Size size)
	{
		assert(size > 0);

		for (Size i = 0; i < size - 1; ++i) {
			data_[i].next = i + 1;
		}
		data_[size - 1].next = NULL_IDX;

		first_ = NULL_IDX;
		last_ = NULL_IDX;
		firstFree_ = 0;
	}

	std::unique_ptr<Entry[]> data_;
	Size first_;
	Size last_;
	Size firstFree_;
};

}
