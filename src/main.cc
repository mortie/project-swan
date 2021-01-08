#include <time.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <chrono>
#include <ratio>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <imgui.h>
#include <imgui_sdl/imgui_sdl.h>

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

// ImGUI and SDL have different numbers for mouse buttons
int sdlButtonToImGuiButton(uint8_t button) {
	switch (button) {
	case SDL_BUTTON_LEFT:
		return 0;
	case SDL_BUTTON_RIGHT:
		return 1;
	case SDL_BUTTON_MIDDLE:
		return 2;
	case SDL_BUTTON_X1:
		return 3;
	case SDL_BUTTON_X2:
		return 4;
	default:
		warn << "Unknown mouse button: " << button;
		return 4; // Let's call that X2?
	}
}

int main(int argc, char **argv) {
	uint32_t winFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	uint32_t renderFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	float guiScale = 1;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--lodpi") == 0) {
			winFlags &= ~SDL_WINDOW_ALLOW_HIGHDPI;
		} else if (strcmp(argv[i], "--fullscreen") == 0) {
			winFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		} else if (strcmp(argv[i], "--no-vsync") == 0) {
			renderFlags &= ~SDL_RENDERER_PRESENTVSYNC;
		} else if (strcmp(argv[i], "--vulkan") == 0) {
			winFlags |= SDL_WINDOW_VULKAN;
		} else if (strcmp(argv[i], "--sw-render") == 0) {
			renderFlags &= ~SDL_RENDERER_ACCELERATED;
			renderFlags |= SDL_RENDERER_SOFTWARE;
		} else if (strcmp(argv[i], "--2x") == 0) {
			guiScale = 2;
		} else if (strcmp(argv[i], "--gles") == 0) {
			SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
		} else {
			warn << "Unknown argument: " << argv[i];
		}
	}

	sdlassert(SDL_Init(SDL_INIT_VIDEO) >= 0, "Could not initialize SDL");
	Deferred<SDL_Quit> sdl;

	int imgFlags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgFlags) == imgFlags, "Could not initialize SDL_Image");
	Deferred<IMG_Quit> sdlImage;

	// Create the window
	CPtr<SDL_Window, SDL_DestroyWindow> window(
		SDL_CreateWindow(
			"Project: SWAN",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			(int)(640 * guiScale), (int)(480 * guiScale), winFlags));

	// Load and display application icon
	CPtr<SDL_Surface, SDL_FreeSurface> icon(
		IMG_Load("assets/icon.png"));
	sdlassert(icon, "Could not load icon");
	SDL_SetWindowIcon(window.get(), icon.get());

	CPtr<SDL_Renderer, SDL_DestroyRenderer> renderer(
		SDL_CreateRenderer(window.get(), -1, renderFlags));
	sdlassert(renderer, "Could not create renderer");
	SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);

	Win win(window.get(), renderer.get(), guiScale);

	// Init ImGUI and ImGUI_SDL
	IMGUI_CHECKVERSION();
	CPtr<ImGuiContext, ImGui::DestroyContext> context(
		ImGui::CreateContext());
	ImGuiSDL::Initialize(renderer.get(), (int)win.getPixSize().x, (int)win.getPixSize().y);
	Deferred<ImGuiSDL::Deinitialize> imguiSDL;
	info << "Initialized with window size " << win.getPixSize();

	// ImGuiIO is to glue SDL and ImGUI together
	ImGuiIO& imguiIO = ImGui::GetIO();
	imguiIO.BackendPlatformName = "imgui_sdl + Project: SWAN";

	// Create a world
	Game game(win);
	std::vector<std::string> mods{ "core.mod" };
	game.createWorld("core::default", mods);

	auto prevTime = std::chrono::steady_clock::now();

	float fpsAcc = 0;
	float tickAcc = 0;

	int fCount = 0;
	int slowFrames = 0;
	while (1) {
		ZoneScopedN("game loop");
		RTClock totalTimeClock;

		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
				break;

			case SDL_WINDOWEVENT:
				if (evt.window.event == SDL_WINDOWEVENT_RESIZED) {
					imguiIO.DisplaySize.x = (float)evt.window.data1;
					imguiIO.DisplaySize.y = (float)evt.window.data2;
					win.onResize(evt.window.data1, evt.window.data2);
				}
				break;

			case SDL_KEYDOWN:
				game.onKeyDown(evt.key.keysym);
				break;

			case SDL_KEYUP:
				game.onKeyUp(evt.key.keysym);
				break;

			case SDL_MOUSEMOTION:
				imguiIO.MousePos.x = (float)evt.motion.x;
				imguiIO.MousePos.y = (float)evt.motion.y;
				if (!imguiIO.WantCaptureMouse)
					game.onMouseMove(evt.motion.x, evt.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
				imguiIO.MouseDown[sdlButtonToImGuiButton(evt.button.button)] = true;
				if (!imguiIO.WantCaptureMouse)
					game.onMouseDown(evt.button.x, evt.button.y, evt.button.button);
				break;

			case SDL_MOUSEBUTTONUP:
				imguiIO.MouseDown[sdlButtonToImGuiButton(evt.button.button)] = false;
				if (!imguiIO.WantCaptureMouse)
					game.onMouseUp(evt.button.x, evt.button.y, evt.button.button);
				break;

			case SDL_MOUSEWHEEL:
				imguiIO.MouseWheel += (float)evt.wheel.y;
				if (!imguiIO.WantCaptureMouse)
					game.onScrollWheel(evt.wheel.y);
				break;
			}
		}

		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> dur(now - prevTime);
		prevTime = now;
		float dt = dur.count();

		// Display FPS
		fpsAcc += dt;
		fCount += 1;
		if (fpsAcc >= 4) {
			info << "FPS: " << fCount / 4.0;
			fpsAcc -= 4;
			fCount = 0;
		}

		// We want to warn if one frame takes over 0.1 seconds...
		if (dt > 0.1) {
			if (slowFrames == 0)
				warn << "Delta time too high! (" << dt << "s)";
			slowFrames += 1;

			// And we never want to do physics as if our one frame is greater than
			// 0.5 seconds.
			if (dt > 0.5)
				dt = 0.5;
		} else if (slowFrames > 0) {
			if (slowFrames > 1)
				warn << slowFrames << " consecutive slow frames.";
			slowFrames = 0;
		}

		// Simple case: we can keep up, only need one physics update
		RTClock updateClock;
		if (dt <= 1 / 25.0) {
			ZoneScopedN("game update");
			game.update(dt);

		// Complex case: run multiple steps this iteration
		} else {
			int count = (int)ceil(dt / (1/30.0));
			float delta = dt / (float)count;
			info << "Delta time " << dt << "s. Running " << count
				<< " updates in one frame, with a delta as if we had "
				<< 1.0 / delta << " FPS.";
			for (int i = 0; i < count; ++i) {
				ZoneScopedN("game update");
				game.update(delta);
			}
		}

		// Tick at a consistent TICK_RATE
		tickAcc += dt;
		while (tickAcc >= 1.0 / TICK_RATE) {
			ZoneScopedN("game tick");
			tickAcc -= 1.0 / TICK_RATE;
			RTClock tickClock;
			game.tick(1.0 / TICK_RATE);
		}

		{
			auto [r, g, b, a] = game.backgroundColor();
			RenderDrawColor c(renderer.get(), r, g, b, a);
			SDL_RenderClear(renderer.get());
		}

		// ImGUI
		imguiIO.DeltaTime = dt;
		ImGui::NewFrame();

		{
			ZoneScopedN("game draw");
			RTClock drawClock;
			game.draw();
		}

		// Render ImGUI
		{
			ZoneScopedN("imgui render");
			ImGui::Render();
			ImGuiSDL::Render(ImGui::GetDrawData());
		}

		RTClock presentClock;
		{
			ZoneScopedN("render present");
			SDL_RenderPresent(renderer.get());
		}
		FrameMark
	}

exit:
	return EXIT_SUCCESS;
}
