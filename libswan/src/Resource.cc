#include "Resource.h"

#include <stdio.h>
#include <SDL2/SDL_image.h>

#include "common.h"
#include "Game.h"
#include "Win.h"

namespace Swan {

ImageResource::ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		const std::string &path, int frame_height) {

	surface_.reset(IMG_Load(path.c_str()));
	if (surface_ == nullptr) {
		fprintf(stderr, "Loading %s failed: %s\n", path.c_str(), SDL_GetError());

		surface_.reset(SDL_CreateRGBSurface(
			0, TILE_SIZE, TILE_SIZE, 32, 0, 0, 0, 0));

		SDL_FillRect(surface_.get(), NULL, SDL_MapRGB(surface_->format,
			PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE));
	}

	if (frame_height < 0)
		frame_height = surface_->h;

	texture_.reset(SDL_CreateTexture(
			renderer, surface_->format->format, SDL_TEXTUREACCESS_STATIC,
			surface_->w, frame_height));

	frame_height_ = frame_height;
	num_frames_ = surface_->h / frame_height_;
	name_ = name;
}

ImageResource::ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		int w, int h, uint8_t r, uint8_t g, uint8_t b) {

	surface_.reset(SDL_CreateRGBSurface(
		0, TILE_SIZE, TILE_SIZE, 32, 0, 0, 0, 0));
	SDL_FillRect(surface_.get(), NULL, SDL_MapRGB(surface_->format, r, g, b));
	texture_.reset(SDL_CreateTexture(
		renderer, surface_->format->format, SDL_TEXTUREACCESS_STATIC, w, h));

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

std::unique_ptr<ImageResource> ImageResource::createInvalid(Context &ctx) {
	return std::make_unique<ImageResource>(
		ctx.game.win_.renderer_, "@internal::invalid", TILE_SIZE, TILE_SIZE,
		PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE);
}

}
