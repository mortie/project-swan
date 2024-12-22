#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace SwanCommon {

// Inspired by:
// https://stackoverflow.com/questions/34596768
struct StringHash {
	using hash_type = std::hash<std::string_view>;
	using is_transparent = void;

	std::size_t operator()(const char *str) const { return hash_type{}(str); }
	std::size_t operator()(std::string_view str) const { return hash_type{}(str); }
	std::size_t operator()(const std::string &str) const { return hash_type{}(str); }
};

template <typename T>
using HashMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;

}
