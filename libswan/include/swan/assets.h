#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <stdint.h>

#include "util.h"
#include "common.h"

namespace Swan {

extern std::string assetBasePath;

struct ImageAsset {
	int width;
	int frameHeight;
	int frameCount;
	int repeatFrom;
	std::unique_ptr<unsigned char[]> data;
};

struct SoundAsset {
	float *l = nullptr, *r = nullptr;
	std::unique_ptr<float[]> data;
	size_t length = 0;
};

Result<ImageAsset> loadImageAsset(
	const HashMap<std::string> &modPaths,
	std::string path, std::optional<int> defaultSize = {});

Result<SoundAsset> loadSoundAsset(
	const HashMap<std::string> &modPaths,
	std::string path);

}
