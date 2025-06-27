#pragma once

#include <charconv>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstddef>
#include <cstdint>
#include <string.h>

namespace Swan {

inline constexpr uint32_t random(uint32_t x)
{
	x ^= 0x55555555u;
	x = ((x >> 16u) ^ x) * 0x45d9f3bu;
	x = ((x >> 16u) ^ x) * 0x45d9f3bu;
	x = (x >> 16u) ^ x;
	return x;
}

inline uint32_t random()
{
	static uint32_t seed = 0;

	seed = random(seed);
	return seed;
}

inline float randfloat()
{
	return (random() & 0xffff) / (float)0xffff;
}

template<typename T, typename U>
inline constexpr T lerp(T a, T b, U t, U max = 1)
{
	t /= max;
	return (a * (1 - t)) + (b * t);
}

// Inherit from this class to make a class non-copyable
class NonCopyable {
public:
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable(NonCopyable &&) noexcept = default;
	NonCopyable &operator=(const NonCopyable &) = delete;
	NonCopyable &operator=(NonCopyable &&) = default;

protected:
	NonCopyable() = default;
	~NonCopyable() = default;
};

// Take a deleter function, turn it into a class with an operator() for unique_ptr
template<typename T, void(*Func)(T *)>
class CPtrDeleter {
public:
	void operator()(T *ptr)
	{
		Func(ptr);
	}
};

// This is just a bit nicer to use than using unique_ptr directly
template<typename T, void(*Func)(T *)>
using CPtr = std::unique_ptr<T, CPtrDeleter<T, Func>>;

// Free memory with 'free'
template<typename T>
class MallocedDeleter {
public:
	void operator()(T *ptr)
	{
		free((void *)ptr);
	}
};

// This is just a bit nicer to use than using unique_ptr directly
template<typename T>
using MallocedPtr = std::unique_ptr<T, MallocedDeleter<T>>;

template<typename F>
class Defer {
public:
	Defer(F f)
		: f_(f)
	{}

	~Defer()
	{
		f_();
	}

private:
	F f_;
};

#define IMPL_SWAN_DEFER_1(x, y) x ## y
#define IMPL_SWAN_DEFER_2(x, y) IMPL_SWAN_DEFER_1(x, y)
#define IMPL_SWAN_DEFER_3(x) IMPL_SWAN_DEFER_2(x, __COUNTER__)

/// Run some code after the end of the block.
#define SWAN_DEFER(code) auto IMPL_SWAN_DEFER_3(_defer_) = ::Swan::Defer([&]() {code;})

inline struct ResultOk {} Ok;
inline struct ResultErr {} Err;

// Result type for potential errors
template<typename T, typename Err = std::string>
class Result {
public:
	Result(ResultOk, T &&val): isOk_(true), v_(ResultOk{}, std::move(val))
	{}
	Result(ResultErr, Err &&err): isOk_(false), v_(ResultErr{}, std::move(err))
	{}

	Result(const Result &other): isOk_(other.isOk_)
	{
		if (isOk_) {
			new (&v_.val) T(other.v_.val);
		}
		else {
			new (&v_.err) T(other.v_.err);
		}
	}

	Result(Result &&other): isOk_(other.isOk_)
	{
		if (other.isOk_) {
			new (&v_.val) T(std::move(other.v_.val));
		}
		else {
			new (&v_.err) Err(std::move(other.v_.err));
		}
	}

	~Result()
	{
		destroy();
	}

	Result<T, Err> &operator=(const Result<T, Err> &other)
	{
		destroy();
		isOk_ = other.isOk_;
		if (isOk_) {
			new (&v_.val) T(other.v_.val);
		}
		else {
			new (&v_.err) Err(other.v_.err);
		}
		return *this;
	}

	Result<T, Err> &operator=(Result<T, Err> &&other)
	{
		destroy();
		isOk_ = other.isOk_;
		if (other.isOk_) {
			new (&v_.val) T(std::move(other.v_.val));
		}
		else {
			new (&v_.err) Err(std::move(other.v_.err));
		}
		return *this;
	}

	explicit operator bool()
	{
		return isOk_;
	}
	bool isOk()
	{
		return isOk_;
	}

	Err &err()
	{
		return v_.err;
	}

	T &value()
	{
		return v_.val;
	}

	T *operator->()
	{
		return &v_.val;
	}

	T &operator*()
	{
		return v_.val;
	}

private:
	void destroy()
	{
		if (isOk_) {
			v_.val.~T();
		}
		else {
			v_.err.~Err();
		}
	}

	bool isOk_;
	union U {
		U()
		{}
		U(ResultOk, T && val) : val(std::move(val))
		{}
		U(ResultOk, const T &val) : val(val)
		{}
		U(ResultErr, Err && err) : err(std::move(err))
		{}
		U(ResultErr, const Err &err) : err(err)
		{}
		~U()
		{}
		T val;
		Err err;
	} v_;
};

// Calling begin/end is stupid...
template<typename T>
auto callBegin(T &v)
{
	using namespace std;
	return begin(v);
}

template<typename T>
auto callEnd(T &v)
{
	using namespace std;
	return end(v);
}

struct CowStr {
	CowStr(std::string s): v(std::move(s)) {}
	CowStr(std::string_view s): v(s) {}

	std::variant<std::string, std::string_view> v;

	std::string_view str() const
	{
		if (auto *s = std::get_if<std::string>(&v); s) {
			return *s;
		}
		if (auto *s = std::get_if<std::string_view>(&v); s) {
			return *s;
		}
		abort();
	}

	operator std::string_view() const { return str(); }
	const char *data() const { return str().data(); }
	size_t size() const { return str().size(); }
};

template<typename T>
CowStr strify(const T &v)
    requires(std::is_integral_v<T>)
{
	char buf[32];
	auto res = std::to_chars(std::begin(buf), std::end(buf), v);
	if (res.ec == std::errc{}) {
		return std::string(buf, res.ptr);
	} else {
		return std::string_view("<invalid>");
	}
}

template<typename T>
CowStr strify(const T &v)
	requires(!std::is_integral_v<T>)
{
	return std::string_view(v);
}

// Concatinate strings
template<typename ...Args>
inline std::string cat(Args &&... args)
{
	CowStr strs[] = {strify(args)...};

	size_t size = 0;

	for (auto &s: strs) {
		size += s.size();
	}

	std::string buf;
	buf.resize(size);
	size_t idx = 0;
	for (auto &s: strs) {
		memcpy(&buf[idx], s.data(), s.size());
		idx += s.size();
	}

	return buf;
}

template<typename A, typename B>
auto max(A a, B b)
{
	using T = decltype(a + b);
	if (a > b) {
		return T(a);
	}
	return T(b);
}

template<typename Head, typename ...Tail>
auto max(Head head, Tail ... tail)
{
	using T = decltype(head + max(tail ...));
	return Swan::max(T(head), T(Swan::max(tail ...)));
}

template<typename A, typename B>
auto min(A a, B b)
{
	using T = decltype(a + b);
	if (a < b) {
		return T(a);
	}
	return T(b);
}

template<typename Head, typename ...Tail>
auto min(Head head, Tail ... tail)
{
	using T = decltype(head + max(tail ...));
	return Swan::min(T(head), T(Swan::min(tail ...)));
}

template<typename T>
int sign(T val)
{
	if (val < T{0}) {
		return -1;
	}
	if (val > T{0}) {
		return 1;
	}
	return 0;
}

template<size_t N>
struct FixedString {
	char buf[N + 1]{};

	constexpr FixedString(char const* s)
	{
		for (size_t i = 0; i < N; ++i) {
			buf[i] = s[i];
		}
	}

	constexpr operator const char *() const
	{
		return buf;
	}

	constexpr const char *cStr() const
	{
		return buf;
	}
};
template<unsigned N> FixedString(char const (&)[N]) -> FixedString<N - 1>;

}
