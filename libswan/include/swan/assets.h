#include <memory>
#include <unordered_map>
#include <string>

#include "util.h"

namespace Swan {

struct ImageAsset {
	int width;
	int frameHeight;
	int frameCount;
	std::unique_ptr<unsigned char[]> data;
};

Result<ImageAsset> loadImageAsset(
		const std::unordered_map<std::string, std::string> modPaths,
		std::string path);

}
