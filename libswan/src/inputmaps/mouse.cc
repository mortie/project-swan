#include "inputmaps/mouse.h"
#include <swan/HashMap.h>
#include <unordered_map>
#include <string>

namespace Swan {

const HashMap<int> fromName = {
#define X(name, code) {name, code},
#include "mouse.x"
#undef X
};

const std::unordered_map<int, const char *> toName = {
#define X(name, code) {code, name},
#include "mouse.x"
#undef X
};

int mouseButtonFromName(std::string_view name)
{
	auto it = fromName.find(name);
	if (it == fromName.end()) {
		return -1;
	}

	return it->second;
}

std::string_view mouseButtonToName(int axis)
{
	auto it = toName.find(axis);
	if (it == toName.end()) {
		return "<unknown>";
	}

	return it->second;
}

}
