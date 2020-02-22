#include "drawutil.h"

#include <algorithm>
#include <cmath>

#include "gfxutil.h"

namespace Swan {
namespace Draw {

static Uint8 linearLine(float from, float to, float frac) {
	return (Uint8)std::clamp(to * frac + from * (1 - frac), 0.0f, 255.0f);
}

static SDL_Color linearColor(SDL_Color from, SDL_Color to, float frac) {
	return {
		.r = linearLine(from.r, to.r, frac),
		.g = linearLine(from.g, to.g, frac),
		.b = linearLine(from.b, to.b, frac),
		.a = linearLine(from.a, to.a, frac),
	};
}

SDL_Color linearGradient(
		float val,
		std::initializer_list<std::pair<float, SDL_Color>> colors) {

	const std::pair<float, SDL_Color> *arr = colors.begin();
	size_t size = colors.size();

	if (val < arr[0].first)
		return arr[0].second;

	for (size_t i = 1; i < size; ++i) {
		if (arr[i].first < val)
			continue;

		auto [fromv, fromc] = arr[i - 1];
		auto [tov, toc] = arr[i];
		float frac = (val - fromv) / (tov - fromv);
		return linearColor(fromc, toc, frac);
	}

	return arr[size - 1].second;
}

void parallaxBackground(
		Win &win, SDL_Texture *tex,
		std::optional<SDL_Rect> srcrect, std::optional<SDL_Rect> destrect,
		float x, float y, float factor) {

	SDL_Renderer *rnd = win.renderer_;

	// We only need to set a clip rect if we have a destrect
	std::optional<RenderClipRect> clip;

	if (!srcrect) {
		Uint32 fmt;
		int access, w, h;
		SDL_QueryTexture(tex, &fmt, &access, &w, &h);
		srcrect = SDL_Rect{ 0, 0, w, h };
	}

	if (destrect) {
		clip.emplace(rnd, &*destrect);
	} else {
		int w, h;
		SDL_RenderGetLogicalSize(rnd, &w, &h);
		destrect = SDL_Rect{ 0, 0, w, h };
	}

	x = (x * win.zoom_) * -factor;
	y = (y * win.zoom_) * -factor;
	SDL_Rect rect{
		0, 0,
		(int)(srcrect->w * win.zoom_),
		(int)(srcrect->h * win.zoom_),
	};

	rect.x = (int)std::floor((int)x % rect.w);
	if (rect.x > 0) rect.x -= rect.w;
	rect.y = (int)std::floor((int)y % rect.h);
	if (rect.y > 0) rect.y -= rect.h;

	int numx = destrect->w / rect.w + 2;
	int numy = destrect->h / rect.h + 2;

	for (int x = 0; x < numx; ++x) {
		for (int y = 0; y < numy; ++y) {
			SDL_Rect r{ rect.x + x * rect.w, rect.y + y * rect.h, rect.w, rect.h };
			SDL_RenderCopy(rnd, tex, &*srcrect, &r);
		}
	}
}

}
}
