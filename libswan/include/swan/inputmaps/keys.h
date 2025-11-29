#pragma once

#include <string_view>

namespace Swan {

int scanCodeFromName(std::string_view name);
std::string_view scanCodeToName(int axis);

}
