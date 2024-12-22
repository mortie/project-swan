#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>

namespace Swan {

template<
	typename Key,
	typename Hash = std::hash<Key>,
	typename KeyEqual = std::equal_to<Key>>
class FastHashSet {
	using Set = FastHashSet<Key, Hash, KeyEqual>;
public:
	FastHashSet() = default;

	FastHashSet(const Set &other)
	{
		*this = other;
	}

	FastHashSet(Set &&other)
	{
		*this = std::move(other);
	}

	~FastHashSet()
	{
		clear();
		free(buffer_);
		free(bits_);
	}

	Set &operator=(const Set &other)
	{
		if (&other == this) {
			return;
		}

		// This is gonna be really slow, but it works
		clear();
		for (size_t index = 0; index < other.cap_; ++index) {
			if (other.isOccupied(index)) {
				insert(other.buffer_[index]);
			}
		}

		return *this;
	}

	Set &operator=(Set &&other)
	{
		if (&other == this) {
			return;
		}

		clear();
		free(buffer_);
		free(bits_);

		cap_ = other.cap_;
		buffer_ = other.buffer_;
		bits_ = other.bits_;
		elementCount_ = other.elementCount_;

		other.cap_ = 0;
		other.buffer_ = nullptr;
		other.bits_ = nullptr;
		other.elementCount_ = 0;
	}

	void clear()
	{
		if (cap_ == 0) {
			return;
		}

		for (size_t i = 0; i < cap_; ++i) {
			if (isOccupied(i)) {
				buffer_[i].~Key();
			}
		}

		memset(bits_, 0, (cap_ / (sizeof(*bits_) * 8) + 1) * sizeof(*bits_));
		elementCount_ = 0;
	}

	void shrinkToFit()
	{
		if (elementCount_ == 0) {
			free(buffer_);
			free(bits_);
			buffer_ = nullptr;
			bits_ = nullptr;
			cap_ = 0;
			return;
		}

		size_t newCap = cap_;
		while (newCap > elementCount_ * 4) {
			newCap /= 2;
		}

		if (newCap == cap_) {
			return;
		}

		rehash(newCap);
	}

	template<typename K>
	void insert(K &&key)
	{
		if (cap_ == 0) {
			rehash(8);
		} else if (elementCount_ >= cap_ / 2) {
			rehash(cap_ == 0 ? 8 : cap_ * 2);
		}

		insertNoResize(std::forward<K>(key));
	}

	void erase(const Key &key)
	{
		if (cap_ == 0) {
			return;
		}

		// Find the key and erase it
		size_t index = Hash{}(key) & (cap_ - 1);
		while (true) {
			if (!isOccupied(index)) {
				return;
			}

			if (KeyEqual{}(buffer_[index], key)) {
				buffer_[index].~Key();
				elementCount_ -= 1;
				break;
			}

			index = (index + 1) & (cap_ - 1);
		}

		// We just made a hole in the table where there was none before!
		// Move everything after it back, until we reach another hole
		// or a fixed point
		while (true) {
			// Last index \ I gave you my heart
			// But the very next day \ You gave it away
			size_t lastIndex = index;
			index = (index + 1) & (cap_ - 1);

			if (!isOccupied(index)) {
				clearOccupied(lastIndex);
				break;
			}

			// Fixed point: an element that's exactly where its hash
			// says it ought to be
			if ((Hash{}(buffer_[index]) & (cap_ - 1)) == index) {
				clearOccupied(lastIndex);
				break;
			}

			new (&buffer_[lastIndex]) Key(std::move(buffer_[index]));
			buffer_[index].~Key();
		}
	}

	bool contains(const Key &key)
	{
		size_t index = Hash{}(key) & (cap_ - 1);
		while (true) {
			if (!isOccupied(index)) {
				return false;
			}

			if (!KeyEqual{}(key, buffer_[index])) {
				index = (index + 1) & (cap_ - 1);
				continue;
			}

			return true;
		}
	}

private:
	template<typename K>
	void insertNoResize(K &&key)
	{
		size_t index = Hash{}(key) & (cap_ - 1);
		while (true) {
			if (isOccupied(index)) {
				if (KeyEqual{}(key, buffer_[index])) {
					return;
				}

				index = (index + 1) & (cap_ - 1);
				continue;
			}

			new (&buffer_[index]) Key(std::forward<K>(key));
			setOccupied(index);
			elementCount_ += 1;
			return;
		}
	}

	void rehash(size_t newCap)
	{
		size_t oldCap = cap_;
		Key *oldBuffer = buffer_;
		unsigned long long *oldBits = bits_;

		cap_ = newCap;
		buffer_ = (Key *)malloc(sizeof(*buffer_) * newCap);
		bits_ = (unsigned long long *)calloc(newCap / (sizeof(*bits_) * 8) + 1, sizeof(*bits_));
		elementCount_ = 0;

		for (size_t index = 0; index < oldCap; ++index) {
			bool occupied = getBit(oldBits, index);
			if (!occupied) {
				continue;
			}

			insertNoResize(std::move(oldBuffer[index]));
			oldBuffer[index].~Key();
		}

		free(oldBuffer);
		free(oldBits);
	}

	bool isOccupied(size_t index)
	{
		return getBit(bits_, index);
	}

	void setOccupied(size_t index)
	{
		setBit(bits_, index);
	}

	void clearOccupied(size_t index)
	{
		clearBit(bits_, index);
	}

	static bool getBit(unsigned long long *bits, size_t bit)
	{
		size_t index = bit / (sizeof(*bits) * 8);
		auto mask = (unsigned long long)1 << (bit % (sizeof(*bits) * 8));
		return bits[index] & mask;
	}

	static void setBit(unsigned long long *bits, size_t bit)
	{
		size_t index = bit / (sizeof(*bits) * 8);
		auto mask = (unsigned long long)1 << (bit % (sizeof(*bits) * 8));
		bits[index] |= mask;
	}

	static void clearBit(unsigned long long *bits, size_t bit)
	{
		size_t index = bit / (sizeof(*bits) * 8);
		auto mask = (unsigned long long)1 << (bit % (sizeof(*bits) * 8));
		bits[index] &= ~mask;
	}

	size_t cap_ = 0;
	Key *buffer_ = nullptr;
	unsigned long long *bits_ = nullptr;
	size_t elementCount_;
};

}
