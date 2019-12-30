#include <SDL2/SDL.h>
#include <ostream>

namespace Swan {

inline std::ostream &operator<<(std::ostream &os, const SDL_Rect &rect) {
	os
		<< "SDL_Rect(" << rect.x << ", " << rect.y << ", "
		<< rect.w << ", " << rect.h << ")";
	return os;
}

}
