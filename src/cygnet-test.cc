#include <cygnet/Context.h>
#include <cygnet/Window.h>
#include <cygnet/Renderer.h>
#include <swan-common/constants.h>

#include <time.h>
#include <stdint.h>
#include <iostream>
#include <SDL_image.h>
#include <SDL.h>

double getTime() {
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	return tv.tv_sec + tv.tv_nsec / 1000000000.0;
}

void addTile(Cygnet::Renderer &rnd, const char *path) {
	static size_t id = 0;
	SDL_Surface *surf = IMG_Load(path);
	rnd.registerTileTexture(id++, surf->pixels, surf->pitch * surf->h);
	SDL_FreeSurface(surf);
}

Cygnet::RenderSprite loadSprite(Cygnet::Renderer &rnd, const char *path) {
	SDL_Surface *surf = IMG_Load(path);
	auto sprite = rnd.createSprite(surf->pixels, surf->w, surf->h);
	SDL_FreeSurface(surf);
	return sprite;
}

int main() {
	Cygnet::Context ctx;
	IMG_Init(IMG_INIT_PNG);
	Cygnet::Window win("Cygnet Test", 680, 680);
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

	Cygnet::RenderSprite playerSprite = loadSprite(rnd, "core.mod/assets/entity/player-running.png");

	Cygnet::RenderChunk chunk;
	{
		uint16_t tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT];
		memset(tiles, 0, sizeof(tiles));
		tiles[0] = 1;
		tiles[1] = 2;
		tiles[2] = 3;
		chunk = rnd.createChunk(tiles);
	}

	Cygnet::RenderCamera cam = {
		.pos = { 0, 0 },
		.size = win.size(),
		.zoom = 1,
	};

	float lol = 0;

	bool keys[512] = { 0 };

	double acc = 0;
	double prevTime = getTime() - 1/60.0;
	int frames = 0;
	float x = 0;

	while (true) {
		double currTime = getTime();
		double dt = currTime - prevTime;
		prevTime = currTime;
		acc += dt;

		frames += 1;
		if (acc >= 2) {
			std::cerr << "FPS: " << (frames / 2.0) << '\n';
			acc -= 2;
			frames = 0;
		}

		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;

			case SDL_WINDOWEVENT:
				switch (evt.window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					win.onResize(evt.window.data1, evt.window.data2);
					cam.size = win.size();
					break;
				}
				break;

			case SDL_MOUSEWHEEL:
				cam.zoom += evt.wheel.y * 0.1 * cam.zoom;
				break;

			case SDL_KEYDOWN:
				keys[evt.key.keysym.scancode] = true;
				break;

			case SDL_KEYUP:
				keys[evt.key.keysym.scancode] = false;
				break;
			}
		}

		if (keys[SDL_SCANCODE_A]) {
			cam.pos.x -= 1 * dt;
		}
		if (keys[SDL_SCANCODE_D]) {
			cam.pos.x += 1 * dt;
		}
		if (keys[SDL_SCANCODE_W]) {
			cam.pos.y -= 1 * dt;
		}
		if (keys[SDL_SCANCODE_S]) {
			cam.pos.y += 1 * dt;
		}

		lol += 1 * dt;
		rnd.modifyChunk(chunk, { 0, 0 }, (int)lol % 6);
		rnd.modifyChunk(chunk, { 4, 4 }, ((int)(lol / 2) + 3) % 6);
		rnd.modifyChunk(chunk, { 3, 2 }, ((int)(lol * 1.5) + 7) % 6);

		rnd.drawChunk(chunk, { 0, 0 });

		x += dt;
		rnd.drawSprite(playerSprite, { x, 0 });

		win.clear();
		rnd.draw(cam);
		win.flip();
	}

exit:
	IMG_Quit();
}
