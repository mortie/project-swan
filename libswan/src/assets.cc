#include "assets.h"

#include "log.h"
#include "util.h"

#include <cstdio>
#include <limits> // IWYU pragma: keep -- needed for cpptoml.h
#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <stb/stb_image.h>
#include <stb/stb_vorbis.h>
#include <cpptoml.h>
#include <string.h>
#include <fstream>

namespace Swan {

std::string assetBasePath = ".";

static void applyHflip(ImageAsset &asset)
{
	size_t rowWidth = asset.width * 4;
	unsigned char tmp[4];

	for (int y = 0; y < asset.frameHeight * asset.frameCount; ++y) {
		for (int x = 0; x < asset.width / 2; ++x) {
			unsigned char *a =
				asset.data.get() + y * rowWidth + x * 4;
			unsigned char *b =
				asset.data.get() + y * rowWidth + (asset.width - x - 1) * 4;
			memcpy(tmp, a, 4);
			memcpy(a, b, 4);
			memcpy(b, tmp, 4);
		}
	}
}

static void applyVflip(ImageAsset &asset)
{
	size_t rowWidth = asset.width * 4;
	unsigned char tmp[4];

	for (int frameIdx = 0; frameIdx < asset.frameCount; ++frameIdx) {
		unsigned char *frame =
			asset.data.get() + (rowWidth * asset.frameHeight * frameIdx);
		for (int y = 0; y < asset.frameHeight / 2; ++y) {
			for (int x = 0; x < asset.width; ++x) {
				unsigned char *a =
					frame + y * rowWidth + x * 4;
				unsigned char *b =
					frame + (asset.frameHeight - y - 1) * rowWidth + x * 4;
				memcpy(tmp, a, 4);
				memcpy(a, b, 4);
				memcpy(b, tmp, 4);
			}
		}
	}
}

static void applyTranspose(ImageAsset &asset)
{
	if (asset.width != asset.frameHeight) {
		warn << "Can't transpose non-square frames";
		return;
	}

	size_t rowWidth = asset.width * 4;
	char tmp[4];
	for (int frameIdx = 0; frameIdx < asset.frameCount; ++frameIdx) {
		unsigned char *frame =
			asset.data.get() + (rowWidth * asset.frameHeight * frameIdx);
		for (int y = 0; y < asset.frameHeight; ++y) {
			for (int x = y + 1; x < asset.width; ++x) {
				unsigned char *a =
					frame + y * rowWidth + x * 4;
				unsigned char *b =
					frame + x * rowWidth + y * 4;
				memcpy(tmp, a, 4);
				memcpy(a, b, 4);
				memcpy(b, tmp, 4);
			}
		}
	}
}

static void collapseGrid(ImageAsset &asset, int frameWidth)
{
	int nx = asset.width / frameWidth;
	int ny = asset.frameCount;
	auto newData = std::make_unique<unsigned char[]>(
		size_t(nx) * frameWidth * size_t(ny) * asset.frameHeight * 4);
	size_t destRowIdx = 0;
	for (int fy = 0; fy < ny; ++fy) {
		unsigned char *srcRow = &asset.data[fy * asset.width * asset.frameHeight * 4];
		for (int fx = 0; fx < nx; ++fx) {
			unsigned char *srcCell = &srcRow[fx * frameWidth * 4];
			for (int y = 0; y < asset.frameHeight; ++y) {
				unsigned char *src = &srcCell[y * asset.width * 4];
				unsigned char *dest = &newData[destRowIdx++ * frameWidth * 4];
				memcpy(dest, src, frameWidth * 4);
			}
		}
	}

	asset.data = std::move(newData);
	asset.width = frameWidth;
	asset.frameCount = nx * ny;
}

static void makeVariant(
	std::string &path, ImageAsset &asset,
	std::vector<std::string> &ops, const std::string &name)
{
	for (const auto &op: ops) {
		if (op == "hflip") {
			applyHflip(asset);
		}
		else if (op == "vflip") {
			applyVflip(asset);
		}
		else if (op == "transpose") {
			applyTranspose(asset);
		}
		else if (op == "rotate90") {
			applyTranspose(asset);
			applyVflip(asset);
		}
		else if (op == "rotate180") {
			applyHflip(asset);
			applyVflip(asset);
		}
		else if (op == "rotate270") {
			applyTranspose(asset);
			applyHflip(asset);
		}
		else {
			warn
				<< path << ": Unknown operation '" << op
				<< "' for variant '" << name << "'";
		}
	}
}

Result<ImageAsset> loadImageAsset(
	const HashMap<std::string> &modPaths,
	std::string path)
{
	auto sep = path.find("::");

	if (sep == std::string::npos) {
		return {Err, "No '::' mod separator"};
	}

	auto modPart = path.substr(0, sep);
	auto pathPart = path.substr(sep + 2, path.size() - sep - 2);

	std::optional<std::string> variantName;
	sep = pathPart.find("::");
	if (sep != std::string::npos) {
		variantName = pathPart.substr(sep + 2, pathPart.size() - sep - 2);
		pathPart = pathPart.substr(0, sep);
	}

	auto modPath = modPaths.find(modPart);
	if (modPath == modPaths.end()) {
		return {Err, cat("No mod named '", modPart, "'")};
	}

	std::string assetPath;
	if (modPath->second[0] == '/') {
		assetPath = cat(modPath->second, "/assets/", pathPart);
	} else {
		assetPath = cat(
				assetBasePath, "/", modPath->second, "/assets/", pathPart);
	}

	std::string pngPath = cat(assetPath, ".png");
	std::string tomlPath = cat(assetPath, ".toml");
	if (!std::filesystem::exists(tomlPath)) {
		tomlPath = cat(assetPath, "/index.toml");
	}

	std::optional<std::string> variantPngPath;
	if (variantName) {
		variantPngPath = cat(assetPath, "/", variantName.value(), ".png");
	}

	std::shared_ptr<cpptoml::table> toml;
	std::shared_ptr<cpptoml::table> variants;
	std::vector<std::string> variant;

	// Load TOML if it exists
	std::ifstream tomlFile(tomlPath);
	if (tomlFile) {
		cpptoml::parser parser(tomlFile);
		try {
			toml = parser.parse();
		} catch (cpptoml::parse_exception &exc) {
			return {Err, cat("Failed to parse toml file ", tomlPath, ": ", exc.what())};
		}

		variants = toml->get_table("variants");
		std::shared_ptr<cpptoml::array> tomlVariant;

		if (variants && variantName) {
			tomlVariant = variants->get_array(*variantName);
		}

		if (tomlVariant) {
			for (auto part: *tomlVariant) {
				auto tomlStr = part->as<std::string>();
				if (!tomlStr) {
					continue;
				}

				variant.push_back(std::move(tomlStr->get()));
			}
		}
	}
	else if (errno != ENOENT) {
		return {Err, cat("Couldn't open ", tomlPath, ": ", strerror(errno))};
	}

	if (variantPngPath && !std::filesystem::exists(*variantPngPath)) {
		variantPngPath = std::nullopt;
	}

	if (variant.size() > 0 && variant[0].starts_with("img:")) {
		std::string path = std::move(variant[0]).substr(4, variant[0].npos);
		variantPngPath = cat(assetPath, "/", path);
		variant.erase(variant.begin());
	}

	int w, h;
	MallocedPtr<unsigned char> buffer;
	if (variantPngPath) {
		buffer.reset(stbi_load(variantPngPath->c_str(), &w, &h, nullptr, 4));
		if (!buffer) {
			return {Err, cat("Loading image ", *variantPngPath, " failed")};
		}
	}
	else {
		buffer.reset(stbi_load(pngPath.c_str(), &w, &h, nullptr, 4));
		if (!buffer) {
			return {Err, cat("Loading image ", pngPath, " failed")};
		}
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

	if (toml) {
		frameHeight = toml->get_as<int>("height").value_or(frameHeight);
		repeatFrom = toml->get_as<int>("repeatFrom").value_or(repeatFrom);
	}

	ImageAsset asset{
		.width = w,
		.frameHeight = frameHeight,
		.frameCount = h / frameHeight,
		.repeatFrom = repeatFrom,
		.data = std::move(bufferCopy),
	};

	if (toml) {
		auto frameWidth = toml->get_as<int>("width");
		if (frameWidth) {
			collapseGrid(asset, *frameWidth);
		}
	}

	if (variant.size() > 0) {
		makeVariant(path, asset, variant, *variantName);
	}

	return {Ok, std::move(asset)};
}

Result<SoundAsset> loadSoundAsset(
	const HashMap<std::string> &modPaths,
	std::string path)
{
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

	std::string assetPath;
	if (modPath->second[0] == '/') {
		assetPath = cat(modPath->second, "/assets/", pathPart);
	} else {
		assetPath = cat(
			assetBasePath, "/", modPath->second, "/assets/", pathPart);
	}
	std::string oggPath = cat(assetPath, ".ogg");

	int err;
	stb_vorbis *vorbis = stb_vorbis_open_filename(
		oggPath.c_str(), &err, nullptr);
	if (!vorbis) {
		return {Err, cat("Couldn't open ", oggPath, ": ", err)};
	}

	SWAN_DEFER(stb_vorbis_close(vorbis));

	stb_vorbis_info info = stb_vorbis_get_info(vorbis);
	unsigned int samples = stb_vorbis_stream_length_in_samples(vorbis);

	SoundAsset asset;
	asset.length = samples;
	if (info.channels == 1) {
		asset.data = std::make_unique<float[]>(samples);
		asset.l = asset.data.get();
		asset.r = asset.data.get();
	}
	else if (info.channels == 2) {
		asset.data = std::make_unique<float[]>(samples * 2);
		asset.l = asset.data.get();
		asset.r = asset.data.get() + samples;
	}
	else {
		return {Err, cat("Invalid channel count: ", info.channels)};
	}

	float *bufs[] = {asset.l, asset.r};
	int n = stb_vorbis_get_samples_float(vorbis, info.channels, bufs, samples);
	if (n != (int)samples) {
		return {Err, cat("Invalid sample count: ", n, ", expected ", samples)};
	}

	return {Ok, std::move(asset)};
}

}
