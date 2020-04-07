#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <unordered_map>

#include "common.h"

namespace Swan {

class ImageResource {
public:
	ImageResource(
		SDL_Renderer *renderer, const std::string &modpath, const std::string &id);
	ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		int w, int h, uint8_t r, uint8_t g, uint8_t b);

	void tick(float dt);

	SDL_Rect frameRect(int frame = -1) const {
		if (frame == -1) frame = frame_;
		return SDL_Rect{ 0, frame_height_ * frame, surface_->w, frame_height_ };
	}

	static std::unique_ptr<ImageResource> createInvalid(Win &win);

	std::unique_ptr<SDL_Surface, void (*)(SDL_Surface *)> surface_{nullptr, &SDL_FreeSurface};
	std::unique_ptr<SDL_Texture, void (*)(SDL_Texture *)> texture_{nullptr, &SDL_DestroyTexture};
	int frame_height_;
	int num_frames_;
	std::string name_;
	int frame_ = 0;

private:
	float switch_interval_ = 1;
	float switch_timer_ = switch_interval_;
};

class ResourceManager {
public:
	ResourceManager(Win &win);

	void tick(float dt);

	ImageResource &getImage(const std::string &name) const;
	void addImage(std::unique_ptr<ImageResource> img) { images_[img->name_] = std::move(img); }

private:
	std::unordered_map<std::string, std::unique_ptr<ImageResource>> images_;
};

}
