#include "Resource.h"

#include <stdio.h>
#include <SDL2/SDL_image.h>

#include "log.h"
#include "common.h"
#include "Game.h"
#include "Win.h"

namespace Swan {

ImageResource::ImageResource(SDL_Renderer *renderer, const Builder &builder) {
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

	surface_.reset(IMG_Load((builder.modpath + "/assets/" + builder.path).c_str()));

	// If we have a surface, and it's the wrong pixel format, convert it
	if (surface_ && surface_->format->format != format) {
		info
			<< "  " << builder.name << ": Converting from "
			<< SDL_GetPixelFormatName(surface_->format->format) << " to "
			<< SDL_GetPixelFormatName(format);
		surface_.reset(SDL_ConvertSurfaceFormat(surface_.get(), format, 0));
	}

	// If we don't have a surface yet (either loading or conversion failed),
	// create a placeholder
	if (!surface_) {
		warn << "Loading image " << builder.name << " failed: " << SDL_GetError();

		surface_.reset(SDL_CreateRGBSurface(
			0, TILE_SIZE, TILE_SIZE, bpp, rmask, gmask, bmask, amask));
		SDL_FillRect(surface_.get(), NULL, SDL_MapRGB(surface_->format,
			PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE));
	}

	frame_height_ = builder.frame_height;
	if (frame_height_ < 0)
		frame_height_ = surface_->h;

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surface_.get()));
	if (!texture_) {
		panic << "CreateTexture failed: " << SDL_GetError();
		abort();
	}

	num_frames_ = surface_->h / frame_height_;
	name_ = builder.name;
}

ImageResource::ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		int w, int h, uint8_t r, uint8_t g, uint8_t b) {

	surface_.reset(SDL_CreateRGBSurface(
		0, TILE_SIZE, TILE_SIZE, 32, 0, 0, 0, 0));
	SDL_FillRect(surface_.get(), NULL, SDL_MapRGB(surface_->format, r, g, b));

	texture_.reset(SDL_CreateTextureFromSurface(renderer, surface_.get()));
	if (!texture_) {
		panic << "CreateTexture failed: " << SDL_GetError();
		abort();
	}

	frame_height_ = h;
	num_frames_ = 1;
	name_ = name;
}

void ImageResource::tick(float dt) {
	switch_timer_ -= dt;
	if (switch_timer_ <= 0) {
		switch_timer_ += switch_interval_;
		frame_ += 1;
		if (frame_ >= num_frames_)
			frame_ = 0;
	}
}

std::unique_ptr<ImageResource> ImageResource::createInvalid(Win &win) {
	return std::make_unique<ImageResource>(
		win.renderer_, "@internal::invalid", TILE_SIZE, TILE_SIZE,
		PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE);
}

ResourceManager::ResourceManager(Win &win) {
	addImage(ImageResource::createInvalid(win));
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
		return getImage("@internal::invalid");
	}
	return *it->second;
}

}
