#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <memory>
#include <chrono>
#include <ratio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <swan/common.h>
#include <swan/World.h>
#include <swan/Game.h>
#include <swan/Timer.h>
#include <swan/Win.h>

using namespace Swan;

#define errassert(expr, str, errfn) do { \
	if (!(expr)) { \
		fprintf(stderr, "%s: %s\n", str, errfn()); \
		return EXIT_FAILURE; \
	} \
} while (0)

#define sdlassert(expr, str) errassert(expr, str, SDL_GetError);
#define imgassert(expr, str) errassert(expr, str, IMG_GetError);

template<typename T>
using DeleteFunc = void (*)(T *);

int main() {
	sdlassert(SDL_Init(SDL_INIT_VIDEO) >= 0, "Could not initialize SDL");
	std::unique_ptr<void, DeleteFunc<void>> sdl(nullptr, [](void *){ SDL_Quit(); });

	int imgflags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgflags) == imgflags, "Could not initialize SDL_Image");
	std::unique_ptr<void, DeleteFunc<void>> sdl_image(nullptr, [](void *){ IMG_Quit(); });

	std::unique_ptr<SDL_Window, DeleteFunc<SDL_Window>> window(
		SDL_CreateWindow(
			"Project: SWAN",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE),
		SDL_DestroyWindow);

	std::unique_ptr<SDL_Renderer, DeleteFunc<SDL_Renderer>> renderer(
		SDL_CreateRenderer(
			window.get(),  -1, SDL_RENDERER_ACCELERATED),
		SDL_DestroyRenderer);
	sdlassert(renderer, "Could not create renderer\n");

	Win win(renderer.get());

	Game game(win);

	auto prevTime = std::chrono::steady_clock::now();

	float fpsAcc = 0;
	float tickAcc = 0;
	int fcount = 0;
	int slowFrames = 0;
	while (1) {
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> dur(prevTime - now);
		prevTime = now;
		float dt = dur.count();

		// Display FPS
		fpsAcc += dt;
		fcount += 1;
		if (fpsAcc >= 4) {
			fprintf(stderr, "FPS: %.3f\n", fcount / 4.0);
			fpsAcc -= 4;
			fcount = 0;
		}

		game.update(dt);

		if (dt > 0.1) {
			if (slowFrames == 0)
				fprintf(stderr, "Warning: delta time is too high! (%.3fs).\n", dt);
			slowFrames += 1;
		} else {
			if (slowFrames > 0) {
				if (slowFrames > 1)
					fprintf(stderr, "%i consecutive slow frames.\n", slowFrames);
				slowFrames = 0;
			}

			tickAcc += dt;
			while (tickAcc >= 1.0 / TICK_RATE) {
				tickAcc -= 1.0 / TICK_RATE;
				game.tick(1.0 / TICK_RATE);
			}
		}

		game.draw();
		SDL_UpdateWindowSurface(window.get());
	}

	return EXIT_SUCCESS;
}
