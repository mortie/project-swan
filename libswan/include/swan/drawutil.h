#pragma once

#include <SDL.h>
#include <optional>

#include "Win.h"

namespace Swan {
namespace Draw {

SDL_Color linearColor(SDL_Color from, SDL_Color to, float frac);

void parallaxBackground(
		Win &win, SDL_Texture *tex,
		std::optional<SDL_Rect> srcrect, std::optional<SDL_Rect> destrect,
		float x, float y, float factor);

}
}
