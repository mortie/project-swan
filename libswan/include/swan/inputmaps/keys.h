#pragma once

#include <string_view>

namespace Swan {

int keyboardKeyFromName(std::string_view name);
std::string_view keyboardKeyToName(int axis);

}
