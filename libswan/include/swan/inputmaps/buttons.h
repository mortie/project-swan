#include <swan/HashMap.h>
#include <unordered_map>
#include <string>
#include <string_view>

namespace Swan {

extern const HashMap<int> gamepadButtonFromName;
extern const std::unordered_map<int, std::string_view> gamepadButtonToName;

}
