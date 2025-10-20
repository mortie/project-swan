#include "inputmaps/axes.h"

namespace Swan {

const HashMap<int> gamepadAxisFromName = {
#define X(name, code) {name, code},
#include "axes.x"
#undef X
};

const std::unordered_map<int, std::string_view> gamepadAxisToName = {
#define X(name, code) {code, name},
#include "axes.x"
#undef X
};

}
