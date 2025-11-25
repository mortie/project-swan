#include "inputmaps/mouse.h"

namespace Swan {

const HashMap<int> mouseButtonFromName = {
#define X(name, code) {name, code},
#include "mouse.x"
#undef X
};

const std::unordered_map<int, std::string_view> mouseButtonToName = {
#define X(name, code) {code, name},
#include "mouse.x"
#undef X
};

}
