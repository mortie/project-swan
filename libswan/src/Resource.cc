#include "Resource.h"

#include <stdio.h>
#include <SDL_image.h>
#include <regex>
#include <cpptoml.h>
#include <string.h>
#include <errno.h>

#include "log.h"
#include "common.h"
#include "Win.h"

namespace Swan {

ImageResource::ImageResource(
		SDL_Renderer *renderer, const std::string &modpath, const std::string &id) {
	static std::regex first_part_re("^.*?/");

	SDL_RendererInfo rinfo;
	if (SDL_GetRendererInfo(renderer, &rinfo) < 0) {
		panic << "GetRendererInfo failed: " << SDL_GetError();
		abort();
	}

	uint32_t format = rinfo.texture_formats[0];
	int bpp = 32;
	uint32_t rmask, gmask, bmask, amask;
	if (SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask) < 0) {
		panic << "PixelFormatEnumToMasks failed: " << SDL_GetError();
		abort();
	}

	std::string assetpath = modpath + "/assets/" +
		std::regex_replace(id, first_part_re, "");

	surface_.reset(IMG_Load((assetpath + ".png").c_str()));

	// If we don't have a surface yet (either loading or conversion failed),
	// create a placeholder
	if (!surface_) {
		warn << "Loading image " << id << " failed: " << SDL_GetError();

		surface_.reset(SDL_CreateRGBSurface(
			0, TILE_SIZE, TILE_SIZE, bpp, rmask, gmask, bmask, amask));
		SDL_FillRect(surface_.get(), NULL, SDL_MapRGB(surface_->format,
			PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE));
	}

	frameHeight_ = 32;

	// Load TOML if it exists
	errno = ENOENT; // I don't know if ifstream is guaranteed to set errno
	std::ifstream tomlfile(assetpath + ".toml");
	if (tomlfile) {
		cpptoml::parser parser(tomlfile);
		try {
			auto toml = parser.parse();
			frameHeight_ = toml->get_as<int>("height").value_or(frameHeight_);
		} catch (cpptoml::parse_exception &exc) {
			warn << "Failed to parse toml file " << assetpath << ".toml: "
				<< exc.what();
		}
	} else if (errno != ENOENT) {
		warn << "Couldn't open " << assetpath << ".toml: " << strerror(errno);
	}

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surface_.get()));
	if (!texture_) {
		panic << "CreateTexture failed: " << SDL_GetError();
		abort();
	}

	numFrames_ = surface_->h / frameHeight_;
	name_ = id;
}

ImageResource::ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {

	surface_.reset(SDL_CreateRGBSurface(
		0, TILE_SIZE, TILE_SIZE, 32,
		0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff));
	SDL_FillRect(surface_.get(), NULL, SDL_MapRGBA(surface_->format, r, g, b, a));

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surface_.get()));
	if (!texture_) {
		panic << "CreateTexture failed: " << SDL_GetError();
		abort();
	}

	frameHeight_ = h;
	numFrames_ = 1;
	name_ = name;
}

void ImageResource::tick(float dt) {
	switchTimer_ -= dt;
	if (switchTimer_ <= 0) {
		switchTimer_ += switchInterval_;
		frame_ += 1;
		if (frame_ >= numFrames_)
			frame_ = 0;
	}
}

ResourceManager::ResourceManager(Win &win) {
	addImage(std::make_unique<ImageResource>(
		win.renderer_, "@::invalid", TILE_SIZE, TILE_SIZE,
		PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE));
	addImage(std::make_unique<ImageResource>(
		win.renderer_, "@::air", TILE_SIZE, TILE_SIZE,
		0, 0, 0, 0));
}

void ResourceManager::tick(float dt) {
	for (auto &[k, v]: images_) {
		v->tick(dt);
	}
}

ImageResource &ResourceManager::getImage(const std::string &name) const {
	auto it = images_.find(name);
	if (it == end(images_)) {
		warn << "Couldn't find image " << name << "!";
		return getImage("@::invalid");
	}
	return *it->second;
}

}
