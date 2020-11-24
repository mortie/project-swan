#include <cygnet/Context.h>
#include <cygnet/Window.h>

#include <SDL.h>

int main() {
	Cygnet::Context ctx;
	Cygnet::Window win("Cygnet Test", 640, 480);

	while (true) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
			}
		}

		win.flip();
	}

exit:
	;
}
