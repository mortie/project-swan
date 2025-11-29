#pragma once

#include <string_view>

namespace Swan {

int mouseButtonFromName(std::string_view name);
std::string_view mouseButtonToName(int axis);

}
