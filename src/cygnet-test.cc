#include <cygnet/Context.h>
#include <cygnet/Window.h>
#include <cygnet/Renderer.h>
#include <swan-common/constants.h>

#include <stdint.h>
#include <SDL_image.h>
#include <SDL.h>

void addTile(Cygnet::Renderer &rnd, const char *path) {
	static size_t id = 0;
	SDL_Surface *surf = IMG_Load(path);
	rnd.registerTileTexture(id++, surf->pixels, surf->pitch * surf->h * 4);
	SDL_FreeSurface(surf);
}

int main() {
	Cygnet::Context ctx;
	IMG_Init(IMG_INIT_PNG);
	//Cygnet::Window win("Cygnet Test", 640, 480);
	Cygnet::Window win("Cygnet Test", 1280, 256);
	Cygnet::Renderer rnd;

	for (auto path: {
		"core.mod/assets/tile/dirt.png",
		"core.mod/assets/tile/grass.png",
		"core.mod/assets/tile/leaves.png",
		"core.mod/assets/tile/stone.png",
		"core.mod/assets/tile/torch.png",
		"core.mod/assets/tile/tree-trunk.png",
	}) addTile(rnd, path);
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
	IMG_Quit();
}
