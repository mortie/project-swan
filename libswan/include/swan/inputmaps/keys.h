#include <swan/HashMap.h>
#include <unordered_map>
#include <string>
#include <string_view>

namespace Swan {

extern const HashMap<int> scanCodeFromName;
extern const std::unordered_map<int, std::string_view> scanCodeToName;

}
