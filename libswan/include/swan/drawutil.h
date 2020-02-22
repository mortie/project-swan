#pragma once

#include <SDL.h>
#include <optional>
#include <initializer_list>
#include <utility>

#include "Win.h"

namespace Swan {
namespace Draw {

SDL_Color linearGradient(
		float val, std::initializer_list<std::pair<float, SDL_Color>> colors);

void parallaxBackground(
		Win &win, SDL_Texture *tex,
		std::optional<SDL_Rect> srcrect, std::optional<SDL_Rect> destrect,
		float x, float y, float factor);

}
}
