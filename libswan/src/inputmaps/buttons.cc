#include "inputmaps/buttons.h"
#include <swan/HashMap.h>
#include <unordered_map>
#include <string>
#include <string_view>

namespace Swan {

static const HashMap<int> fromName = {
#define X(name, code) {name, code},
#include "buttons.x"
#undef X
};

static const std::unordered_map<int, std::string_view> toName = {
#define X(name, code) {code, name},
#include "buttons.x"
#undef X
};

int gamepadButtonFromName(std::string_view name)
{
	auto it = fromName.find(name);
	if (it == fromName.end()) {
		return -1;
	}

	return it->second;
}

std::string_view gamepadButtonToName(int axis)
{
	auto it = toName.find(axis);
	if (it == toName.end()) {
		return "<unknown>";
	}

	return it->second;
}

}
