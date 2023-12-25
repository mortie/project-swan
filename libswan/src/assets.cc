#include "assets.h"

#include <limits> // IWYU pragma: keep -- needed for cpptoml.h
#include <stb/stb_image.h>
#include <cpptoml.h>
#include <string.h>

namespace Swan {

std::string assetBasePath = ".";

Result<ImageAsset> loadImageAsset(
		const std::unordered_map<std::string, std::string> &modPaths,
		std::string path) {
	auto sep = path.find("::");
	if (sep == std::string::npos) {
		return {Err, "No '::' mod separator"};
	}

	auto modPart = path.substr(0, sep);
	auto pathPart = path.substr(sep + 2, path.size() - sep - 2);

	auto modPath = modPaths.find(modPart);
	if (modPath == modPaths.end()) {
		return {Err, cat("No mod named '", modPart, "'")};
	}

	std::string assetPath = cat(assetBasePath, "/", modPath->second, "/assets/", pathPart);
	std::string pngPath = cat(assetPath, ".png");
	std::string tomlPath = cat(assetPath, ".toml");

	int w, h;
	MallocedPtr<unsigned char> buffer{stbi_load(pngPath.c_str(), &w, &h, nullptr, 4)};
	if (!buffer) {
		return {Err, cat("Loading image ", pngPath, " failed")};
	}

	// Need to make a new buffer in order to be able to use a plain unique_ptr
	// which uses the delete[] deleter.
	// TODO(perf): Re-structure stuff so that we can avoid the copy
	size_t size = (size_t)w * (size_t)h * 4;
	auto bufferCopy = std::make_unique<unsigned char[]>(size);
	memcpy(bufferCopy.get(), buffer.get(), size);
	buffer.reset();

	int frameHeight = h;
	int repeatFrom = 0;

	// Load TOML if it exists
	std::ifstream tomlFile(tomlPath);
	if (tomlFile) {
		cpptoml::parser parser(tomlFile);
		try {
			auto toml = parser.parse();
			frameHeight = toml->get_as<int>("height").value_or(frameHeight);
			repeatFrom = toml->get_as<int>("repeatFrom").value_or(repeatFrom);
		} catch (cpptoml::parse_exception &exc) {
			return {Err, cat("Failed to parse toml file ", tomlPath, ": ", exc.what())};
		}
	} else if (errno != ENOENT) {
		return {Err, cat("Couldn't open ", tomlPath, ": ", strerror(errno))};
	}

	ImageAsset asset{
		.width = w,
		.frameHeight = frameHeight,
		.frameCount = h / frameHeight,
		.repeatFrom = repeatFrom,
		.data = std::move(bufferCopy),
	};

	return {Ok, std::move(asset)};
}

}
