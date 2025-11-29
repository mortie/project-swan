#include "inputmaps/axes.h"
#include <swan/HashMap.h>
#include <unordered_map>
#include <string>

namespace Swan {

static const HashMap<int> fromName = {
#define X(name, code) {name, code},
#include "axes.x"
#undef X
};

static const std::unordered_map<int, std::string_view> toName = {
#define X(name, code) {code, name},
#include "axes.x"
#undef X
};

int gamepadAxisFromName(std::string_view name)
{
	auto it = fromName.find(name);
	if (it == fromName.end()) {
		return -1;
	}

	return it->second;
}

std::string_view gamepadAxisToName(int axis)
{
	auto it = toName.find(axis);
	if (it == toName.end()) {
		return "<unknown>";
	}

	return it->second;
}

}
