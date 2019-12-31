#include <time.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <chrono>
#include <ratio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <swan/swan.h>
#include <swan/util.h>

using namespace Swan;

#define errassert(expr, str, errfn) do { \
	if (!(expr)) { \
		panic << (str) << ": " << errfn(); \
		return EXIT_FAILURE; \
	} \
} while (0)

#define sdlassert(expr, str) errassert(expr, str, SDL_GetError);
#define imgassert(expr, str) errassert(expr, str, IMG_GetError);

template<typename T>
using DeleteFunc = void (*)(T *);

int main() {
	sdlassert(SDL_Init(SDL_INIT_VIDEO) >= 0, "Could not initialize SDL");
	auto sdl = makeDeferred([] { SDL_Quit(); });

	int imgflags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgflags) == imgflags, "Could not initialize SDL_Image");
	auto sdl_image = makeDeferred([] { IMG_Quit(); });

	auto window = makeRaiiPtr(
		SDL_CreateWindow(
			"Project: SWAN",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI),
		SDL_DestroyWindow);

	auto renderer = makeRaiiPtr(
		SDL_CreateRenderer(
			window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
		SDL_DestroyRenderer);
	sdlassert(renderer, "Could not create renderer");

	Win win(window.get(), renderer.get());

	Game game(win);
	std::vector<std::unique_ptr<Mod>> mods;
	mods.push_back(game.loadMod("core.mod"));
	game.createWorld("core::default", std::move(mods));

	auto prevTime = std::chrono::steady_clock::now();

	float fpsAcc = 0;
	float tickAcc = 0;
	int fcount = 0;
	int slowFrames = 0;
	while (1) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
				break;

			case SDL_KEYDOWN:
				game.onKeyDown(evt.key.keysym);
				break;

			case SDL_KEYUP:
				game.onKeyUp(evt.key.keysym);
				break;

			case SDL_MOUSEMOTION:
				game.onMouseMove(evt.motion.x, evt.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				game.onMouseDown(evt.button.x, evt.button.y, evt.button.button);
				break;

			case SDL_MOUSEBUTTONUP:
				game.onMouseUp(evt.button.x, evt.button.y, evt.button.button);
				break;
			}
		}

		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> dur(now - prevTime);
		prevTime = now;
		float dt = dur.count();

		// Display FPS
		fpsAcc += dt;
		fcount += 1;
		if (fpsAcc >= 4) {
			info << "FPS: " << fcount / 4.0;
			fpsAcc -= 4;
			fcount = 0;
		}


		if (dt > 0.1) {
			if (slowFrames == 0)
				warn << "Delta time too high! (" << dt << "s)";
			slowFrames += 1;
			dt = 0.1;
		}

		game.update(dt);

		if (slowFrames > 0) {
			if (slowFrames > 1)
				warn << slowFrames << " consecutive slow frames.";
			slowFrames = 0;
		}

		tickAcc += dt;
		while (tickAcc >= 1.0 / TICK_RATE) {
			tickAcc -= 1.0 / TICK_RATE;
			game.tick(1.0 / TICK_RATE);
		}

		SDL_RenderClear(renderer.get());
		game.draw();
		SDL_RenderPresent(renderer.get());
	}

exit:
	return EXIT_SUCCESS;
}
