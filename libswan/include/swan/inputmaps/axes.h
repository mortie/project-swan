#pragma once

#include <string_view>

namespace Swan {

int gamepadAxisFromName(std::string_view name);
std::string_view gamepadAxisToName(int axis);

}
