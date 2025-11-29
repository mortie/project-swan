#pragma once

#include <string_view>

namespace Swan {

int gamepadButtonFromName(std::string_view name);
std::string_view gamepadButtonToName(int axis);

}
