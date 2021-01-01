#include <cygnet/Context.h>
#include <cygnet/Window.h>
#include <cygnet/Renderer.h>
#include <cygnet/ResourceManager.h>
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

void addTile(Cygnet::ResourceBuilder &builder, const char *path) {
	static size_t id = 0;
	SDL_Surface *surf = IMG_Load(path);
	builder.addTile(id++, surf->pixels);
	SDL_FreeSurface(surf);
}

Cygnet::RenderSprite loadSprite(Cygnet::ResourceBuilder &builder, const char *path, int fh) {
	SDL_Surface *surf = IMG_Load(path);
	auto sprite = builder.addSprite(path, surf->pixels, surf->w, surf->h, fh);
	SDL_FreeSurface(surf);
	return sprite;
}

Cygnet::RenderSprite loadSprite(Cygnet::ResourceBuilder &builder, const char *path) {
	SDL_Surface *surf = IMG_Load(path);
	auto sprite = builder.addSprite(path, surf->pixels, surf->w, surf->h);
	SDL_FreeSurface(surf);
	return sprite;
}

int main() {
	Cygnet::Context ctx;
	IMG_Init(IMG_INIT_PNG);
	Cygnet::Window win("Cygnet Test", 680, 680);
	Cygnet::Renderer rnd;
	Cygnet::ResourceBuilder rbuilder(rnd);

	for (auto path: {
		"core.mod/assets/tile/dirt.png",
		"core.mod/assets/tile/grass.png",
		"core.mod/assets/tile/leaves.png",
		"core.mod/assets/tile/stone.png",
		"core.mod/assets/tile/torch.png",
		"core.mod/assets/tile/tree-trunk.png",
	}) addTile(rbuilder, path);

	unsigned char lolTexture[32*32*4*3];
	for (size_t i = 0; i < 3; ++i) {
		int col = 100 * i + 50;;
		for (size_t y = 0; y < 32; ++y) {
			for (size_t x = 0; x < 32; ++x) {
				lolTexture[i * 32 * 32 * 4 + y * 32 * 4 + x * 4 + 0] = col;
				lolTexture[i * 32 * 32 * 4 + y * 32 * 4 + x * 4 + 1] = col;
				lolTexture[i * 32 * 32 * 4 + y * 32 * 4 + x * 4 + 2] = col;
				lolTexture[i * 32 * 32 * 4 + y * 32 * 4 + x * 4 + 3] = 255;
			}
		}
	}
	rbuilder.addTile(10, lolTexture, 3);

	Cygnet::RenderSprite playerSprite = loadSprite(
			rbuilder, "core.mod/assets/entity/player-still.png", 64);

	Cygnet::ResourceManager resources(std::move(rbuilder));

	Cygnet::RenderChunk chunk;
	{
		uint16_t tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT];
		memset(tiles, 0, sizeof(tiles));
		tiles[0] = 1;
		tiles[1] = 2;
		tiles[2] = 3;
		tiles[10] = 10;
		chunk = rnd.createChunk(tiles);
	}

	Cygnet::RenderCamera cam = {
		.pos = { 0, 0 },
		.size = win.size(),
		.zoom = 1,
	};

	float lol = 0;

	bool keys[512] = { 0 };

	double tileAnimAcc = 0;
	double fpsAcc = 0;
	double prevTime = getTime() - 1/60.0;
	int frames = 0;
	float x = 0, y = 0;

	while (true) {
		double currTime = getTime();
		double dt = currTime - prevTime;
		prevTime = currTime;
		lol += dt;

		fpsAcc += dt;
		frames += 1;
		if (fpsAcc >= 2) {
			std::cerr << "FPS: " << (frames / 2.0) << '\n';
			fpsAcc -= 2;
			frames = 0;
		}

		tileAnimAcc += dt;
		if (tileAnimAcc >= 0.5) {
			resources.tick();
			tileAnimAcc -= 0.5;
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
			x -= 1 * dt;
		}
		if (keys[SDL_SCANCODE_D]) {
			x += 1 * dt;
		}
		if (keys[SDL_SCANCODE_W]) {
			y -= 1 * dt;
		}
		if (keys[SDL_SCANCODE_S]) {
			y += 1 * dt;
		}

		rnd.drawChunk(chunk, { 0, 0 });

		rnd.drawSprite(playerSprite, { x, y }, (int)lol % 2);
		cam.pos = { x, y };

		win.clear();
		rnd.draw(cam);
		win.flip();
	}

exit:
	IMG_Quit();
}
