#include "assets.h"

#include "log.h"
#include "util.h"

#include <cstdio>
#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <stb/stb_image.h>
#include <stb/stb_vorbis.h>
#include <cpptoml.h>
#include <string.h>
#include <fstream>
#include <cygnet/ResourceManager.h>

namespace Swan {

static void collapseGrid(
	unsigned char *data, int width, int height,
	int frameWidth, int frameHeight,
	int *newWidth, int *newFrameCount)
{
	int nx = width / frameWidth;
	int ny = height / frameHeight;
	size_t nbytes = size_t(nx) * frameWidth * size_t(ny) * frameHeight * 4;
	auto newData = std::make_unique<unsigned char[]>(nbytes);
	size_t destRowIdx = 0;
	for (int fy = 0; fy < ny; ++fy) {
		unsigned char *srcRow = &data[fy * width * frameHeight * 4];
		for (int fx = 0; fx < nx; ++fx) {
			unsigned char *srcCell = &srcRow[fx * frameWidth * 4];
			for (int y = 0; y < frameHeight; ++y) {
				unsigned char *src = &srcCell[y * width * 4];
				unsigned char *dest = &newData[destRowIdx++ * frameWidth * 4];
				memcpy(dest, src, frameWidth * 4);
			}
		}
	}

	memcpy(data, newData.get(), nbytes);
	*newWidth = frameWidth;
	*newFrameCount = nx * ny;
}

static void loadSpriteAsset(
	std::string name, const char *path, cpptoml::table *toml,
	Cygnet::ResourceBuilder &builder)
{
	if (builder.hasSprite(name)) {
		info << "Asset " << path << " was overwritten.";
		return;
	}

	int w, h;
	MallocedPtr<unsigned char> buffer(stbi_load(path, &w, &h, nullptr, 4));
	if (!buffer) {
		warn << "Failed to load " << path;
		return;
	}

	Cygnet::ResourceBuilder::SpriteMeta meta = {
		.width = w,
		.height = h,
		.frameHeight = h,
		.repeatFrom = 0,
	};

	if (toml) {
		meta.frameHeight = toml->get_as<int>("height").value_or(meta.frameHeight);
		meta.repeatFrom = toml->get_as<int>("repeat-from").value_or(0);

		auto frameWidth = toml->get_as<int>("width");
		int frameCount;
		if (frameWidth) {
			collapseGrid(
				buffer.get(), w, h,
				*frameWidth, meta.frameHeight,
				&meta.width, &frameCount);
			meta.height = frameCount * meta.frameHeight;
		}
	}

	builder.addSprite(std::move(name), buffer.get(), meta);
}

static void loadSoundAsset(
	std::string name, const char *path, HashMap<SoundAsset> &assets)
{
	if (assets.contains(name)) {
		info << "Asset " << path << " was overwritten.";
		return;
	}

	int err;
	stb_vorbis *vorbis = stb_vorbis_open_filename(
			path, &err, nullptr);
	if (!vorbis) {
		warn << "Couldn't open " << path << ": " << err;
		return;
	}

	stb_vorbis_info vinfo = stb_vorbis_get_info(vorbis);
	if (vinfo.sample_rate != 48000) {
		warn << path << ": Sample rate is " << vinfo.sample_rate << ", expected 48000.";
	}

	unsigned int samples = stb_vorbis_stream_length_in_samples(vorbis);
	SoundAsset asset;
	asset.length = samples;
	if (vinfo.channels == 1) {
		asset.data = std::make_unique<float[]>(samples);
		asset.l = asset.data.get();
		asset.r = asset.data.get();
	} else if (vinfo.channels == 2) {
		asset.data = std::make_unique<float[]>(samples * 2);
		asset.l = asset.data.get();
		asset.r = asset.data.get() + samples;
	} else {
		warn << path << ": Unexpected channel count: " << vinfo.channels;
		return;
	}

	float *bufs[] = {asset.l, asset.r};
	int n = stb_vorbis_get_samples_float(vorbis, vinfo.channels, bufs, samples);
	if (n != (int)samples) {
		warn << "Invalid sample count: " << n << ", expected " << samples;
		return;
	}

	info << "Registering sound asset: " << name << " -> " << path;
	assets[std::move(name)] = std::move(asset);
}

void loadSpriteAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder)
{
	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newBase = cat(base, it.path().filename(), "/");
			std::string newPath = cat(path, "/", it.path().filename());
			loadSpriteAssets(std::move(newBase), std::move(newPath), builder);
			continue;
		}

		auto ext = it.path().filename().extension();
		if (ext == ".txt" || ext == ".toml") {
			continue;
		} else if (ext != ".png") {
			warn << it.path() << ": Unknown extension " << ext;
			continue;
		}

		std::string name = cat(base, it.path().filename().stem());
		std::string newPath = cat(path, "/", it.path().filename());
		std::string tomlPath = cat(path, "/", it.path().filename().stem(), ".toml");

		std::shared_ptr<cpptoml::table> toml;
		if (std::filesystem::exists(tomlPath)) {
			toml = cpptoml::parse_file(tomlPath);
		}

		loadSpriteAsset(std::move(name), newPath.c_str(), toml.get(), builder);
	}
}

void loadTileOrImageAssets(
	std::string_view base, std::string_view path,
	HashMap<ImageAsset> &assets)
{
}

void loadSoundAssets(
	std::string base, std::string path,
	HashMap<SoundAsset> &assets)
{
	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newBase = cat(base, it.path().filename(), "/");
			std::string newPath = cat(path, "/", it.path().filename());
			loadSoundAssets(std::move(newBase), std::move(newPath), assets);
			continue;
		}

		auto ext = it.path().filename().extension();
		if (ext == ".txt") {
			continue;
		} else if (ext != ".ogg") {
			warn << it.path() << ": Unknown extension " << ext;
			continue;
		}

		std::string name = cat(base, it.path().filename().stem());
		std::string newPath = cat(path, "/", it.path().filename());
		loadSoundAsset(std::move(name), newPath.c_str(), assets);
	}
}

}
