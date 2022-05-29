#include "assets.h"

#include <SDL_image.h>
#include <cpptoml.h>
#include <string.h>

namespace Swan {

Result<ImageAsset> loadImageAsset(
		const std::unordered_map<std::string, std::string> modPaths,
		std::string path) {
	auto sep = path.find("::");
	if (sep == std::string::npos) {
		return {Err, "No '::' mod separator"};
	}

	auto modPart = path.substr(0, sep);
	auto pathPart = path.substr(sep + 2, path.size() - sep - 2);

	auto modPath = modPaths.find(modPart);
	if (modPath == modPaths.end()) {
		return {Err, "No mod named '" + modPart + '\''};
	}

	std::string assetPath = modPath->second + "/assets/" + pathPart;
	std::string pngPath = assetPath + ".png";
	std::string tomlPath = assetPath + ".toml";

	CPtr<SDL_Surface, SDL_FreeSurface> surface(IMG_Load(pngPath.c_str()));
	if (!surface) {
		return {Err, "Loading image " + pngPath + " failed: " + SDL_GetError()};
	}

	int frameHeight = surface->h;

	// Load TOML if it exists
	errno = ENOENT; // I don't know if ifstream is guaranteed to set errno
	std::ifstream tomlFile(tomlPath);
	if (tomlFile) {
		cpptoml::parser parser(tomlFile);
		try {
			auto toml = parser.parse();
			frameHeight = toml->get_as<int>("height").value_or(frameHeight);
		} catch (cpptoml::parse_exception &exc) {
			return {Err, "Failed to parse toml file " + tomlPath + ": " + exc.what()};
		}
	} else if (errno != ENOENT) {
		return {Err, "Couldn't open " + tomlPath + ": " + strerror(errno)};
	}

	ImageAsset asset{
		.width = surface->w,
		.frameHeight = frameHeight,
		.frameCount = surface->h / frameHeight,
		.data = std::make_unique<unsigned char[]>(surface->w * surface->h * 4),
	};

	// TODO: Pixel formats?
	for (size_t y = 0; y < (size_t)surface->h; ++y) {
		unsigned char *src = (unsigned char *)surface->pixels + y * surface->pitch;
		unsigned char *dest = asset.data.get() + y * surface->w * 4;
		memcpy(dest, src, surface->w * 4);
	}

	return {Ok, std::move(asset)};
}

}
