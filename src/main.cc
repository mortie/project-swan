#include <time.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <chrono>
#include <ratio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>

#include <swan/swan.h>

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

int main(int argc, char **argv) {
	uint32_t winflags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	uint32_t renderflags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--lodpi") == 0) {
			winflags &= ~SDL_WINDOW_ALLOW_HIGHDPI;
		} else if (strcmp(argv[i], "--fullscreen") == 0) {
			winflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else if (strcmp(argv[i], "--no-vsync") == 0) {
			renderflags &= ~SDL_RENDERER_PRESENTVSYNC;
		} else if (strcmp(argv[i], "--vulkan") == 0) {
			winflags |= SDL_WINDOW_VULKAN;
		} else if (strcmp(argv[i], "--sw-render") == 0) {
			renderflags &= ~SDL_RENDERER_ACCELERATED;
			renderflags |= SDL_RENDERER_SOFTWARE;
		} else {
			warn << "Unknown argument: " << argv[i];
		}
	}

	sdlassert(SDL_Init(SDL_INIT_VIDEO) >= 0, "Could not initialize SDL");
	auto sdl = makeDeferred([] { SDL_Quit(); });

	int imgflags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgflags) == imgflags, "Could not initialize SDL_Image");
	auto sdl_image = makeDeferred([] { IMG_Quit(); });

	auto window = makeRaiiPtr(
		SDL_CreateWindow(
			"Project: SWAN",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			640, 480, winflags),
		SDL_DestroyWindow);

	// Load and display application icon
	auto icon = makeRaiiPtr(
		IMG_Load("assets/icon.png"),
		SDL_FreeSurface);
	sdlassert(icon, "Could not load icon");
	SDL_SetWindowIcon(window.get(), icon.get());

	auto renderer = makeRaiiPtr(
		SDL_CreateRenderer(window.get(), -1, renderflags),
		SDL_DestroyRenderer);
	sdlassert(renderer, "Could not create renderer");

	Win win(window.get(), renderer.get());

	Game game(win);
	std::vector<std::unique_ptr<Mod>> mods;
	mods.push_back(game.loadMod("core.mod"));
	game.createWorld("core::default", std::move(mods));

	PerfCounter pcounter;

	auto prev_time = std::chrono::steady_clock::now();

	float fps_acc = 0;
	float tick_acc = 0;

	int fcount = 0;
	int slow_frames = 0;
	while (1) {
		RTClock total_frame_clock;

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
		std::chrono::duration<float> dur(now - prev_time);
		prev_time = now;
		float dt = dur.count();

		// Display FPS
		fps_acc += dt;
		fcount += 1;
		if (fps_acc >= 4) {
			info << "FPS: " << fcount / 4.0;
			fps_acc -= 4;
			fcount = 0;
		}

		// We want to warn if one frame takes over 0.1 seconds...
		if (dt > 0.1) {
			if (slow_frames == 0)
				warn << "Delta time too high! (" << dt << "s)";
			slow_frames += 1;

			// And we never want to do physics as if our one frame is greater than
			// 0.5 seconds.
			if (dt > 0.5)
				dt = 0.5;
		} else if (slow_frames > 0) {
			if (slow_frames > 1)
				warn << slow_frames << " consecutive slow frames.";
			slow_frames = 0;
		}

		// Simple case: we can keep up, only need one physics update
		RTClock update_clock;
		if (dt <= 1 / 25.0) {
			pcounter.countGameUpdatesPerFrame(1);
			game.update(dt);

		// Complex case: run multiple steps this iteration
		} else {
			int count = (int)ceilf(dt / (1/30.0));
			pcounter.countGameUpdatesPerFrame(count);
			float delta = dt / count;
			info << "Delta time " << dt << "s. Running " << count << " updates in one frame, with a delta as if we had " << 1.0 / delta << " FPS.";
			for (int i = 0; i < count; ++i)
				game.update(delta);
		}
		pcounter.countGameUpdate(update_clock.duration());

		// Tick at a consistent TICK_RATE
		tick_acc += dt;
		while (tick_acc >= 1.0 / TICK_RATE) {
			tick_acc -= 1.0 / TICK_RATE;
			RTClock tick_clock;
			game.tick(1.0 / TICK_RATE);
			pcounter.countGameTick(tick_clock.duration());
		}

		SDL_RenderClear(renderer.get());

		RTClock draw_clock;
		game.draw();
		pcounter.countGameDraw(draw_clock.duration());

		pcounter.countFrameTime(total_frame_clock.duration());

		RTClock present_clock;
		SDL_RenderPresent(renderer.get());
		pcounter.countRenderPresent(present_clock.duration());
	}

exit:
	return EXIT_SUCCESS;
}
