#pragma once

#include <new>
#include <stdlib.h>
#include <utility>
#include <iostream>

namespace Swan {

// The initial bytes policy assumes that no T object will ever start with
// 'size' 'empty_byte' bytes. This works for types where, for example,
// the first few bytes are a pointer.
template<typename T, size_t size = sizeof(T), unsigned char empty_byte = 0xFF>
struct SmallOptionalInitialBytesPolicy {
	static_assert(sizeof(T) >= size);
	static void setEmpty(unsigned char *ptr) {
		for (size_t i = 0; i < size; ++i)
			ptr[i] = empty_byte;
	}

	static bool isEmpty(const unsigned char *ptr) {
		for (size_t i = 0; i < size; ++i)
			if (ptr[i] != empty_byte)
				return false;
		return true;
	}
};

// The value policy lets you define a particular T value which can be a sentinel
// and is considered different from valid T values according to T::operator==.
// For example, if T is int, -1 could be a sentinel if only positive values are expected.
template<typename T, T empty_value>
struct SmallOptionalValuePolicy {
	static void setEmpty(unsigned char *ptr) {
		*(T *)ptr = empty_value;
	}

	static bool isEmpty(const unsigned char *ptr) {
		return *(T *)ptr == empty_value;
	}
};

// This is probably UB but I don't care, it avoids wasting 8 bytes per entity
template<typename T, typename Policy = SmallOptionalInitialBytesPolicy<T>>
class SmallOptional {
public:
	SmallOptional() {
		Policy::setEmpty(data_);
	}
	SmallOptional(const T &other) {
		new (data_) T(other);
	}
	SmallOptional(T &&other) noexcept {
		new (data_) T(std::move(other));
	}
	SmallOptional(const SmallOptional<T, Policy> &other) {
		if (other.hasValue()) {
			new (data_) T(*other.get());
		} else {
			Policy::setEmpty(data_);
		}
	}
	SmallOptional(SmallOptional<T, Policy> &&other) noexcept {
		if (other.hasValue()) {
			new (data_) T(std::move(*other));
		} else {
			Policy::setEmpty(data_);
		}
	}
	~SmallOptional() {
		if (hasValue()) {
			get()->~T();
		}
	}

	void reset() {
		if (hasValue()) {
			get()->~T();
			Policy::setEmpty(data_);
		}
	}

	template<typename... Args>
		T &emplace(Args&&... args) {
			if (hasValue()) {
				get()->~T();
			}

			new (data_) T(std::forward<Args>(args)...);
			return *get();
		}

	T *get() {
		return std::launder<T>((T *)data_);
	}
	const T *get() const {
		return std::launder<T>((T *)data_);
	}

	bool hasValue() const {
		return !Policy::isEmpty(data_);
	}

	SmallOptional<T, Policy> &operator=(const T &other) {
		if (hasValue()) {
			*get() = other;
		} else {
			new (data_) T(other);
		}
		return *this;
	}
	SmallOptional<T, Policy> &operator=(const T &&other) noexcept {
		if (hasValue()) {
			*get() = std::move(other);
		} else {
			new (data_) T(std::move(other));
		}
		return *this;
	}
	SmallOptional<T, Policy> &operator=(const SmallOptional<T, Policy> &other) {
		if (other.hasValue()) {
			if (hasValue()) {
				*get() = *other.get();
			} else {
				new (data_) T(*other.get());
			}
		} else {
			reset();
		}
		return *this;
	}
	SmallOptional &operator=(SmallOptional<T, Policy> &&other) noexcept {
		if (other.hasValue()) {
			if (hasValue()) {
				*get() = std::move(*other.get());
			} else {
				new (data_) T(std::move(*other.get()));
			}
		} else {
			reset();
		}
		return *this;
	}

	bool operator==(const SmallOptional<T, Policy> &other) const {
		bool a = hasValue(), b = other.hasValue();
		if (!a && !b) return true;
		if (!a || !b) return false;
		return *get() == *other.get();
	}

	operator bool() const noexcept {
		return !Policy::isEmpty(data_);
	}

	T *operator->() noexcept {
		return get();
	}
	const T *operator->() const noexcept {
		return get();
	}
	T &operator*() noexcept {
		return *get();
	}
	const T &operator*() const noexcept {
		return *get();
	}

private:
	alignas(alignof(T)) unsigned char data_[sizeof(T)];
};

}
