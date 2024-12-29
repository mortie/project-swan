#pragma once

#include <stdint.h>
#include <type_traits>

namespace Swan {

enum class Tool: uint8_t {
	NONE = 0,
	HAND = 1 << 0,
};

class ToolSet {
public:
	constexpr ToolSet() = default;
	constexpr ToolSet(const ToolSet &) = default;
	constexpr ToolSet(Tool value): value_(Int(value)) {}

	constexpr operator bool()
	{
		return value_ != 0;
	}

	constexpr friend ToolSet operator|(const Tool &a, const Tool &b)
	{
		return Tool(Int(a) | Int(b));
	}

	constexpr friend ToolSet operator&(const Tool &a, const Tool &b)
	{
		return Tool(Int(a) & Int(b));
	}

	constexpr friend ToolSet operator|(const ToolSet &a, const Tool &b)
	{
		return Tool(Int(a.value_) | Int(b));
	}

	constexpr friend ToolSet operator&(const ToolSet &a, const Tool &b)
	{
		return Tool(Int(a.value_) & Int(b));
	}

private:
	using Int = std::underlying_type_t<Tool>;

	Int value_ = Int(Tool::NONE);
};

}
