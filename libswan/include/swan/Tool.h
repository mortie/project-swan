#pragma once

#include <stdint.h>
#include <type_traits>

namespace Swan {

enum class Tool: uint8_t {
	NONE = 0,
	HAND = 1 << 0,
	AXE = 1 << 1,
};

class ToolSet {
public:
	constexpr ToolSet() = default;
	constexpr ToolSet(const ToolSet &) = default;
	constexpr ToolSet(Tool value): value_(Int(value)) {}

	ToolSet &operator=(const ToolSet &) = default;

	constexpr operator bool()
	{
		return value_ != 0;
	}

	constexpr bool contains(ToolSet tool)
	{
		return *this & tool;
	}

	constexpr friend ToolSet operator|(const ToolSet &a, const ToolSet &b)
	{
		return Tool(Int(a.value_) | Int(b.value_));
	}

	constexpr friend ToolSet operator&(const ToolSet &a, const ToolSet &b)
	{
		return Tool(Int(a.value_) & Int(b.value_));
	}

private:
	using Int = std::underlying_type_t<Tool>;

	Int value_ = Int(Tool::NONE);
};

}
