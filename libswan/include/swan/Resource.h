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
		SDL_Renderer *renderer, const std::string &name,
		const std::string &path, int frame_height = -1);
	ImageResource(
		SDL_Renderer *renderer, const std::string &name,
		int w, int h, uint8_t r, uint8_t g, uint8_t b);

	void tick(float dt);

	static std::unique_ptr<ImageResource> createInvalid(Context &ctx);

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
	void tick(float dt);

	ImageResource &getImage(const std::string &name) const;
	void addImage(std::string name, std::unique_ptr<ImageResource> img);

	std::unique_ptr<ImageResource> invalid_image_;

private:
	std::unordered_map<std::string, std::unique_ptr<ImageResource>> images_;
};

}