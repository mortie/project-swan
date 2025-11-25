#include "inputmaps/buttons.h"

namespace Swan {

const HashMap<int> gamepadButtonFromName = {
#define X(name, code) {name, code},
#include "buttons.x"
#undef X
};

const std::unordered_map<int, std::string_view> gamepadButtonToName = {
#define X(name, code) {code, name},
#include "buttons.x"
#undef X
};

}
