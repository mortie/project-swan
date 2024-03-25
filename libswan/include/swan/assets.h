#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <stdint.h>

#include "util.h"

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
	float *l, *r;
	std::unique_ptr<float[]> data;
	size_t length;
};

Result<ImageAsset> loadImageAsset(
	const std::unordered_map<std::string, std::string> &modPaths,
	std::string path);

Result<SoundAsset> loadSoundAsset(
	const std::unordered_map<std::string, std::string> &modPaths,
	std::string path);

}
