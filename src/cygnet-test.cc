#include <cygnet/Context.h>
#include <cygnet/Window.h>
#include <cygnet/Renderer.h>
#include <swan-common/constants.h>

#include <stdint.h>
#include <SDL.h>

int main() {
	Cygnet::Context ctx;
	Cygnet::Window win("Cygnet Test", 640, 480);
	Cygnet::Renderer rnd;

	uint32_t img[SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE];
	for (size_t i = 0; i < sizeof(img) / sizeof(*img); ++i) {
		img[i] = 0xff00aaff;
	}
	rnd.registerTileTexture(0, img, sizeof(img));
	rnd.uploadTileTexture();

	while (true) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
			}
		}

		win.clear();
		rnd.draw();
		win.flip();
	}

exit:
	;
}
