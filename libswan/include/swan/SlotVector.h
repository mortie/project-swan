#pragma once

#include <vector>
#include <type_traits>

namespace Swan {

template<typename T>
struct SlotVectorDefaultSentinel {
	static constexpr T sentinel() { return T{}; }
};

template<typename T, T val>
struct SlotVectorValueSentinel {
	static constexpr T sentinel() { return val; }
};

template<typename T, typename Sentinel = SlotVectorDefaultSentinel<T>>
class SlotVector {
public:
	class Iterator {
	public:
		Iterator(SlotVector<T, Sentinel> *vec, size_t idx): vec_(vec), idx_(idx) {}
		Iterator(const Iterator &) = default;

		void seek() {
			size_t size = vec_->vec_.size();
			while (idx_ < size && (*vec_)[idx_] == Sentinel::sentinel())
				idx_ += 1;
		}

		void operator++() {
			idx_ += 1;
			seek();
		}

		T &operator*() {
			return (*vec_)[idx_];
		}

		bool operator==(const Iterator &other) const {
			return idx_ == other.idx_;
		}
		bool operator!=(const Iterator &other) const {
			return !(idx_ == other.idx_);
		}
		size_t operator-(const Iterator &other) const {
			return idx_ - other.idx_;
		}

	private:
		SlotVector<T, Sentinel> *vec_;
		size_t idx_;
	};

	T &operator[](size_t idx) { return vec_[idx]; }
	T &at(size_t idx) { return vec_.at(idx); }
	size_t size() { return vec_.size(); }
	bool empty() { return vec_.size() == free_.size(); }

	Iterator begin() { Iterator it(this, 0); it.seek(); return it; }
	Iterator end() { return Iterator(this, vec_.size()); }

	void clear() noexcept { vec_.clear(); free_.clear(); }

	void shrink_to_fit() {
		vec_.shrink_to_fit();
		free_.shrink_to_fit();
	}

	size_t insert(T val) {
		if (free_.size() > 0) {
			size_t idx = free_.back();
			vec_[idx] = std::move(val);
			free_.pop_back();
			return idx;
		} else {
			size_t idx = vec_.size();
			vec_.push_back(std::move(val));
			return idx;
		}
	}

	template<typename... Args>
	size_t emplace(Args&&... args) {
		if (free_.size() > 0) {
			size_t idx = free_.back();
			T *slot = vec_.data() + idx;
			slot->~T();
			new (slot) T(std::forward<Args>(args)...);
			return idx;
		} else {
			size_t idx = vec_.size();
			vec_.emplace_back(std::forward<Args>(args)...);
			return idx;
		}
	}

	void erase(size_t idx) {
		vec_[idx] = Sentinel::sentinel();
		free_.push_back(idx);
	}

private:
	std::vector<T> vec_;
	std::vector<size_t> free_;
};

}
