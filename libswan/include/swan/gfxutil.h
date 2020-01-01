#include <SDL2/SDL.h>
#include <ostream>

#include "util.h"

namespace Swan {

inline std::ostream &operator<<(std::ostream &os, const SDL_Rect &rect) {
	os
		<< "SDL_Rect(" << rect.x << ", " << rect.y << ", "
		<< rect.w << ", " << rect.h << ")";
	return os;
}

class TexLock {
public:
	TexLock(SDL_Texture *tex, SDL_Rect *rect = nullptr);
	TexLock(const TexLock &) = delete;
	TexLock(TexLock &&lock) noexcept;
	~TexLock();

	int blit(SDL_Rect *destrect, SDL_Surface *srcsurf, SDL_Rect *srcrect = nullptr) {
		return SDL_BlitSurface(srcsurf, srcrect, surf_.get(), destrect);
	}

private:
	SDL_Texture *tex_;
	RaiiPtr<SDL_Surface> surf_ = makeRaiiPtr<SDL_Surface>(nullptr, SDL_FreeSurface);
};

}
