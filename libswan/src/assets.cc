#include "assets.h"

#include "log.h"
#include <swan/util.h>

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

static std::unique_ptr<TileParticles> buildTileParticles(unsigned char *data)
{
	// This code has hard-coded assumptions that tiles are 32x32
	// and particles are 8x8.
	static_assert(TILE_SIZE == 32);
	static_assert(sizeof(TileParticles{}.particles) == 8 * 8 * 4);

	auto particles = std::make_unique<TileParticles>();

	auto averageColor = [&](int x, int y) {
		Cygnet::Color avg = {0, 0, 0, 0};
		float count = 0;

		for (int ry = 0; ry < 4; ++ry) {
			int py = (y * 4) + ry;
			unsigned char *row = &data[py * 32 * 4];
			for (int rx = 0; rx < 4; ++rx) {
				int px = (x * 4) + rx;
				unsigned char *pix = &row[px * 4];
				if (pix[3] == 0) {
					continue;
				}

				avg.r += pix[0];
				avg.g += pix[1];
				avg.b += pix[2];
				avg.a += pix[3];
				count += 1;
			}
		}

		if (count == 0) {
			return Cygnet::ByteColor{0, 0, 0, 0};
		}

		return Cygnet::ByteColor{
			uint8_t(avg.r / count),
			uint8_t(avg.g / count),
			uint8_t(avg.b / count),
			uint8_t(avg.a / count),
		};
	};

	for (int y = 0; y < 8; ++y) {
		for (int x = 0; x < 8; ++x) {
			particles->particles[y][x] = averageColor(x, y);
		}
	}

	return particles;
}

static float findImageYOffset(unsigned char *data)
{
	int y;
	bool done = false;
	for (y = 0; y < TILE_SIZE && !done; ++y) {
		unsigned char *row = &data[(TILE_SIZE - y - 1) * TILE_SIZE * 4];
		for (int x = 0; x < TILE_SIZE; ++x) {
			unsigned char *pix = &row[x * 4];
			if (pix[3] > 0) {
				done = true;
				break;
			}
		}
	}

	return (y - 1) / float(TILE_SIZE);
}

static void applyHflip(unsigned char *data, size_t frameCount)
{
	size_t rowWidth = TILE_SIZE * 4;
	unsigned char tmp[4];

	for (size_t y = 0; y < TILE_SIZE * frameCount; ++y) {
		for (size_t x = 0; x < TILE_SIZE / 2; ++x) {
			unsigned char *a =
				data + y * rowWidth + x * 4;
			unsigned char *b =
				data + y * rowWidth + (TILE_SIZE - x - 1) * 4;
			memcpy(tmp, a, 4);
			memcpy(a, b, 4);
			memcpy(b, tmp, 4);
		}
	}
}

static void applyVflip(unsigned char *data, size_t frameCount)
{
	size_t rowWidth = TILE_SIZE * 4;
	unsigned char tmp[4];

	for (size_t frameIdx = 0; frameIdx < frameCount; ++frameIdx) {
		unsigned char *frame =
			data + (rowWidth * TILE_SIZE * frameIdx);
		for (int y = 0; y < TILE_SIZE / 2; ++y) {
			for (int x = 0; x < TILE_SIZE; ++x) {
				unsigned char *a =
					frame + y * rowWidth + x * 4;
				unsigned char *b =
					frame + (TILE_SIZE - y - 1) * rowWidth + x * 4;
				memcpy(tmp, a, 4);
				memcpy(a, b, 4);
				memcpy(b, tmp, 4);
			}
		}
	}
}

static void applyTranspose(unsigned char *data, size_t frameCount)
{
	size_t rowWidth = TILE_SIZE * 4;
	char tmp[4];
	for (size_t frameIdx = 0; frameIdx < frameCount; ++frameIdx) {
		unsigned char *frame =
			data + (rowWidth * TILE_SIZE * frameIdx);
		for (int y = 0; y < TILE_SIZE; ++y) {
			for (int x = y + 1; x < TILE_SIZE; ++x) {
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

static bool applyOp(
	std::string_view op, unsigned char *data, size_t frameCount)
{
	if (op == "hflip") {
		applyHflip(data, frameCount);
		return true;
	}
	else if (op == "vflip") {
		applyVflip(data, frameCount);
		return true;
	}
	else if (op == "transpose") {
		applyTranspose(data, frameCount);
		return true;
	}
	else if (op == "rotate90") {
		applyTranspose(data, frameCount);
		applyVflip(data, frameCount);
		return true;
	}
	else if (op == "rotate180") {
		applyHflip(data, frameCount);
		applyVflip(data, frameCount);
		return true;
	}
	else if (op == "rotate270") {
		applyTranspose(data, frameCount);
		applyHflip(data, frameCount);
		return true;
	}
	else {
		return false;
	}
}

static void collapseGrid(
	unsigned char *data, int width, int height,
	int frameWidth, int frameHeight,
	int *newFrameCount)
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
	*newFrameCount = nx * ny;
}

static void loadSpriteAsset(
	std::string name, const char *path, cpptoml::table *toml,
	Cygnet::ResourceBuilder &builder)
{
	if (builder.hasSprite(name)) {
		info << "Sprite asset " << path << " was overwritten.";
		return;
	}

	int w, h;
	MallocedPtr<unsigned char> buffer(stbi_load(path, &w, &h, nullptr, 4));
	if (!buffer) {
		warn << "Failed to load asset " << path;
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
				&frameCount);
			meta.width = *frameWidth;
			meta.height = frameCount * meta.frameHeight;
		}
	}

	builder.addSprite(std::move(name), buffer.get(), meta);
}

static void loadTileAsset(
	std::string name, std::string_view dir, cpptoml::table &toml,
	Cygnet::ResourceBuilder &builder, HashMap<TileAssetMeta> &meta)
{
	auto variants = toml.get_table("variants");
	if (!variants) {
		warn << "No variants for "<< name;
		return;
	}

	auto baseName = toml.get_as<std::string>("base");

	MallocedPtr<unsigned char> baseImg;
	int baseFrameCount = 0;

	auto loadImg = [&](std::string_view name, int *fc) -> MallocedPtr<unsigned char> {
		auto path = cat(dir, "/", name);
		int w, h;
		MallocedPtr<unsigned char> buffer(stbi_load(path.c_str(), &w, &h, nullptr, 4));
		if (!buffer) {
			warn << "Failed to load tile " << path;
			return nullptr;
		}

		if (w < TILE_SIZE || w % TILE_SIZE != 0) {
			warn << "Invalid width for " << path << ": " << w;
			return nullptr;
		}

		if (h < TILE_SIZE || h % TILE_SIZE != 0) {
			warn << "Invalid height for " << path << ": " << h;
			return nullptr;
		}

		collapseGrid(buffer.get(), w, h, TILE_SIZE, TILE_SIZE, fc);
		return buffer;
	};

	auto ensureBase = [&]() -> bool {
		if (baseImg) {
			return true;
		}

		if (!baseName) {
			warn << "Variant requires 'img:' for " << name;
			return false;
		}

		baseImg = loadImg(*baseName, &baseFrameCount);
		return baseImg != nullptr;
	};

	for (auto &[variantName, variantToml]: *variants) {
		std::string assetName;
		if (variantName == "default") {
			assetName = name;
		} else {
			assetName = cat(name, "::", variantName);
		}

		if (builder.hasTileAsset(assetName)) {
			info << "Tile asset " << name << " was overwritten.";
			continue;
		}

		MallocedPtr<unsigned char> variantImgBuf;
		unsigned char *variantImg;
		int variantFrameCount = 0;
		auto variantPtr = variantToml->as_array();
		if (!variantPtr) {
			warn << "Invalid variant " << variantName << " for " << name;
			continue;
		}
		auto &variant = *variantPtr;

		std::shared_ptr<cpptoml::value<std::string>> firstVariant;
		if (variant.size() > 0) {
			firstVariant = variant.at(0)->as<std::string>();
			if (!firstVariant) {
				warn
					<< "Invalid variant part in " << variantName
					<< " for " << name << ": " << variant.at(0);
				break;
			}
		}

		size_t startIndex;
		if (firstVariant && firstVariant->get().starts_with(".")) {
			variantImgBuf = loadImg(firstVariant->get(), &variantFrameCount);
			if (!variantImgBuf) {
				continue;
			}

			variantImg = variantImgBuf.get();
			startIndex = 1;
		}
		else if (variant.size() > 0) {
			if (!ensureBase()) {
				continue;
			}

			variantImgBuf.reset((unsigned char *)malloc(
				TILE_SIZE * TILE_SIZE * 4 * baseFrameCount));
			memcpy(
				variantImgBuf.get(), baseImg.get(),
				TILE_SIZE * TILE_SIZE * 4 * baseFrameCount);
			variantImg = variantImgBuf.get();
			variantFrameCount = baseFrameCount;
			startIndex = 0;
		}
		else {
			if (!ensureBase()) {
				continue;
			}

			variantImg = baseImg.get();
			variantFrameCount = baseFrameCount;
			startIndex = 0;
		}

		for (size_t idx = startIndex; idx < variant.size(); ++idx) {
			auto op = variant.at(idx)->as<std::string>();
			if (!op) {
				warn
					<< "Invalid variant operation in " << variantName
					<< " for " << name << ": " << variant.at(idx);
				break;
			}

			if (!applyOp(op->get(), variantImg, variantFrameCount)) {
				warn
					<< "Invalid variant operation " << op->get() << " in "
					<< variantName << " for " << name;
			}
		}

		builder.addTileAsset(assetName, variantImg, variantFrameCount);

		if (variantFrameCount > 0) {
			for (int i = 0; i < variantFrameCount; ++i) {
				unsigned char *data = variantImg + (TILE_SIZE * TILE_SIZE * 4 * i);
				meta[cat(assetName, "@", i)] = TileAssetMeta {
					.yOffset = findImageYOffset(data),
					.particles = buildTileParticles(data),
				};
			}
		}
		meta[std::move(assetName)] = TileAssetMeta {
			.yOffset = findImageYOffset(variantImg),
			.particles = buildTileParticles(variantImg),
		};
	}
}

static void loadSoundAsset(
	std::string name, const char *path, HashMap<SoundAsset> &assets)
{
	if (assets.contains(name)) {
		info << "Sound asset " << path << " was overwritten.";
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

	assets[std::move(name)] = std::move(asset);
}

void loadSpriteAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newPath = cat(path, "/", it.path().filename());
			std::string newBase = cat(base, it.path().filename(), "/");
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
		std::string assetPath = cat(path, "/", it.path().filename());
		std::string tomlPath = cat(path, "/", it.path().filename().stem(), ".toml");

		std::shared_ptr<cpptoml::table> toml;
		if (std::filesystem::exists(tomlPath)) {
			toml = cpptoml::parse_file(tomlPath);
		}

		loadSpriteAsset(std::move(name), assetPath.c_str(), toml.get(), builder);
	}
}

void loadTileAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder, HashMap<TileAssetMeta> &meta)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newPath = cat(path, "/", it.path().filename());

			// Load tile asset from a directory with an index.toml
			std::string tomlPath = cat(newPath, "/index.toml");
			if (std::filesystem::exists(tomlPath)) {
				std::shared_ptr<cpptoml::table> toml;
				toml = cpptoml::parse_file(tomlPath);
				std::string name = cat(base, it.path().filename().stem());
				loadTileAsset(
					std::move(name), newPath, *toml,
					builder, meta);
				continue;
			}

			std::string newBase = cat(base, it.path().filename(), "/");
			loadTileAssets(std::move(newBase), std::move(newPath), builder, meta);
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
		std::string tomlPath = cat(path, "/", it.path().filename().stem(), ".toml");

		std::shared_ptr<cpptoml::table> toml;
		if (std::filesystem::exists(tomlPath)) {
			toml = cpptoml::parse_file(tomlPath);
		} else {
			toml = cpptoml::make_table();
		}

		auto variants = toml->get_table("variants");
		if (!variants) {
			variants = cpptoml::make_table();
			toml->insert("variants", variants);
		}

		if (!variants->contains("default")) {
			variants->insert("default", cpptoml::make_array());
		}

		if (!toml->contains("base")) {
			toml->insert("base", cat(it.path().filename().stem(), ".png"));
		}

		loadTileAsset(std::move(name), path, *toml, builder, meta);
	}
}

void loadSoundAssets(
	std::string base, std::string path,
	HashMap<SoundAsset> &assets)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

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
		std::string assetPath = cat(path, "/", it.path().filename());
		loadSoundAsset(std::move(name), assetPath.c_str(), assets);
	}
}

}
