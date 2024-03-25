#include "assets.h"

#include "log.h"
#include "util.h"

#include <limits> // IWYU pragma: keep -- needed for cpptoml.h
#include <memory>
#include <optional>
#include <string>
#include <filesystem>
#include <stb/stb_image.h>
#include <stb/stb_vorbis.h>
#include <cpptoml.h>
#include <string.h>

namespace Swan {

std::string assetBasePath = ".";

void applyHflip(ImageAsset &asset)
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

void applyVflip(ImageAsset &asset)
{
	size_t rowWidth = asset.width * 4;
	unsigned char tmp[4];

	for (int frameIdx = 0; frameIdx < asset.frameCount; ++frameIdx) {
		unsigned char *frame =
			asset.data.get() + (rowWidth * asset.frameHeight * frameIdx);
		for (int y = 0; y < asset.frameHeight * asset.frameCount; ++y) {
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

void applyTranspose(ImageAsset &asset)
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

static void makeVariant(
	ImageAsset &asset, std::shared_ptr<cpptoml::table> toml, const std::string &name)
{
	if (!toml) {
		warn << "Variant '" << name << "' requested but there's no TOML file";
		return;
	}

	auto variants = toml->get_table("variants");
	if (!variants) {
		warn
			<< "Variant '" << name << "' but "
			<< "there's no variants in the TOML";
		return;
	}

	auto ops = variants->get_array(name);
	if (!ops) {
		warn
			<< "Variant '" << name << "' requested but"
			<< "there's no such variant in the TOML";
		return;
	}

	for (const auto &op: *ops) {
		auto val = op->as<std::string>();
		if (!val) {
			warn << "Operation for variant '" << name << "' is not a string";
			continue;
		}

		auto &str = val->get();
		if (str == "hflip") {
			applyHflip(asset);
		}
		else if (str == "vflip") {
			applyVflip(asset);
		}
		else if (str == "transpose") {
			applyTranspose(asset);
		}
		else {
			warn << "Unknown operation '" << str << "' for variant '" << name << "'";
		}
	}
}

Result<ImageAsset> loadImageAsset(
	const std::unordered_map<std::string, std::string> &modPaths,
	std::string path)
{
	auto sep = path.find("::");

	if (sep == std::string::npos) {
		return {Err, "No '::' mod separator"};
	}

	auto modPart = path.substr(0, sep);
	auto pathPart = path.substr(sep + 2, path.size() - sep - 2);

	std::optional<std::string> variantPart;
	sep = pathPart.find("::");
	if (sep != std::string::npos) {
		variantPart = pathPart.substr(sep + 2, pathPart.size() - sep - 2);
		pathPart = pathPart.substr(0, sep);
	}

	auto modPath = modPaths.find(modPart);
	if (modPath == modPaths.end()) {
		return {Err, cat("No mod named '", modPart, "'")};
	}

	std::string assetPath = cat(
		assetBasePath, "/", modPath->second, "/assets/", pathPart);
	std::string pngPath = cat(assetPath, ".png");
	std::string tomlPath = cat(assetPath, ".toml");

	std::optional<std::string> variantPngPath;
	if (variantPart) {
		variantPngPath = cat(assetPath, "/", variantPart.value(), ".png");
	}

	int w, h;
	MallocedPtr<unsigned char> buffer;
	if (variantPngPath && std::filesystem::exists(variantPngPath.value())) {
		buffer.reset(stbi_load(pngPath.c_str(), &w, &h, nullptr, 4));
		if (!buffer) {
			return {Err, cat("Loading image ", variantPngPath.value(), " failed")};
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
	std::shared_ptr<cpptoml::table> toml;

	// Load TOML if it exists
	std::ifstream tomlFile(tomlPath);
	if (tomlFile) {
		cpptoml::parser parser(tomlFile);
		try {
			toml = parser.parse();
		} catch (cpptoml::parse_exception &exc) {
			return {Err, cat("Failed to parse toml file ", tomlPath, ": ", exc.what())};
		}
	}
	else if (errno != ENOENT) {
		return {Err, cat("Couldn't open ", tomlPath, ": ", strerror(errno))};
	}

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

	if (variantPart) {
		makeVariant(asset, toml, variantPart.value());
	}

	return {Ok, std::move(asset)};
}

Result<SoundAsset> loadSoundAsset(
	const std::unordered_map<std::string, std::string> &modPaths,
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

	std::string assetPath = cat(
		assetBasePath, "/", modPath->second, "/assets/", pathPart);
	std::string oggPath = cat(assetPath, ".ogg");

	int err;
	stb_vorbis *vorbis = stb_vorbis_open_filename(
		oggPath.c_str(), &err, nullptr);
	if (!vorbis) {
		return {Err, cat(
			"Couldn't open ", oggPath, ": ", std::to_string(err))};
	}

	defer(stb_vorbis_close(vorbis));

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
		return {Err, cat(
			"Invalid channel count: ", std::to_string(info.channels))};
	}

	float *bufs[] = {asset.l, asset.r};
	int n = stb_vorbis_get_samples_float(vorbis, info.channels, bufs, samples);
	if (n != samples) {
		return {Err, cat(
			"Invalid sample count: ", std::to_string(n),
			", expected ", std::to_string(samples))};
	}

	return {Ok, std::move(asset)};
}

}
