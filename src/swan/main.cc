#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_net/SDL_net.h>

#include <cstdlib>
#include <memory>
#include <random>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <chrono>

#ifndef __MINGW32__
#include <backward.hpp>
#endif

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <cygnet/gl.h>
#include <cygnet/Renderer.h>
#include <filesystem>

#include <swan/swan.h>
#include <swan/HashMap.h>
#include <swan/assets.h>
#include <swan/GameIO.h>
#include <swan/MPGame.h>

#include "../swan-build/build.h"

struct AppState {
	float pixelRatio = -1;
	int windowWidth = -1;
	int windowHeight = -1;
	bool framebufferSizeDirty = true;

	ImGuiIO imguiIo;
	Swan::CPtr<SDL_Window, SDL_DestroyWindow> window;
	SDL_GLContext glContext;

#ifndef SWAN_HEADLESS
	GLuint globalVao = 0;
#endif

	const char *thumbnailPath = nullptr;
	std::unique_ptr<Swan::GameIO> game;
};

static void onFramebufferSizeChanged(AppState *state)
{
	float pixelRatio = SDL_GetWindowDisplayScale(state->window.get());
	if (state->pixelRatio != pixelRatio) {
		Swan::info << "Window DPI scale: " << pixelRatio;
		state->pixelRatio = pixelRatio;

		ImGui::GetStyle().ScaleAllSizes(pixelRatio);
		ImGui::GetStyle().FontScaleDpi = pixelRatio;

		state->imguiIo.FontGlobalScale = 1.0 / pixelRatio;
		state->imguiIo.Fonts->ClearFonts();
		state->imguiIo.Fonts->AddFontFromFileTTF(
			"assets/NotoSans-Regular.ttf", 17 * pixelRatio);
		state->imguiIo.Fonts->Build();
	}

	int width, height;
	SDL_GetWindowSizeInPixels(state->window.get(), &width, &height);
	bool sizeChanged = (
		width != state->windowWidth ||
		height != state->windowHeight);
	if (sizeChanged) {
		Swan::info << "Viewport size: " << width << 'x' << height;
		glViewport(0, 0, width, height);
		state->windowWidth = width;
		state->windowHeight = height;
		state->game->onViewportSize(width, height);
	}
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		Swan::panic << "Failed to create window: " << SDL_GetError();
		return SDL_APP_FAILURE;
	}

	if (!NET_Init()) {
		Swan::panic << "Failed to initialize SDL3_net.";
		return SDL_APP_FAILURE;
	}

#ifndef __MINGW32__
	backward::SignalHandling sh;
#endif

	std::optional<uint32_t> seedArg;
	const char *worldPath = nullptr;
	std::vector<std::string> modPaths;
	const char *swanRoot = ".";
	bool doCompileMods = true;
	const char *thumbnailPath = nullptr;

	Swan::MPClient::Options multiplayer = {
		.host = "",
		.port = 11216,
		.nick = "dummy",
		.identifier = "dummyidentifier",
	};

	for (int i = 1; i < argc; ++i) {
		std::string_view arg = argv[i];
		if (arg == "--mod") {
			i += 1;
			modPaths.push_back(argv[i]);
		} else if (arg == "--mp-host") {
			i += 1;
			multiplayer.host = argv[i];
		} else if (arg == "--mp-nick") {
			i += 1;
			multiplayer.nick = argv[i];
		} else if (arg == "--mp-port") {
			i += 1;
			multiplayer.port = atoi(argv[i]);
		} else if (arg == "--mp-identifier") {
			i += 1;
			multiplayer.identifier = atoi(argv[i]);
		} else if (arg == "--swan") {
			i += 1;
			swanRoot = argv[i];
		} else if (arg == "--world") {
			i += 1;
			worldPath = argv[i];
		} else if (arg == "--no-compile") {
			doCompileMods = false;
		} else if (arg == "--thumbnail") {
			i += 1;
			thumbnailPath = argv[i];
		} else if (arg == "--seed") {
			i += 1;
			seedArg = uint32_t(std::stoul(argv[i]));
		} else {
			Swan::warn << "Unexpected option: " << arg;
		}
	}

	if (modPaths.empty()) {
		Swan::panic << "Empty mods list!";
		return SDL_APP_FAILURE;
	}

	Swan::HashMap<Swan::ModInfo> mods;
	std::vector<std::string> modIDs;
	for (auto &path: modPaths) {
		auto mod = Swan::ModInfo::parse(path);
		if (!mod) {
			Swan::panic << "Failed to parse mod info for " << path;
			return SDL_APP_FAILURE;
		}

		auto id = Swan::cat(mod->name, "@", mod->version);
		mods[id] = std::move(*mod);
		modIDs.push_back(std::move(id));
	}

#ifdef SWAN_HEADLESS
	if (multiplayer.host != "") {
		Swan::panic << "Can't join a server in headless mode.";
		return 1;
	}

	Swan::info << "Running in headless mode.";
#endif

	auto compileMods = [&]() {
		if (!doCompileMods) {
			return true;
		}

		Swan::ScopedTimer timer("compile mods");

		for (auto &[id, mod]: mods) {
			if (!SwanBuild::build(mod.path.c_str(), swanRoot)) {
				return false;
			}
		}

		return true;
	};
	if (!compileMods()) {
		return SDL_APP_FAILURE;
	}

	auto state = new AppState();
	*appstate = (void *)state;
	state->thumbnailPath = thumbnailPath;

#ifndef SWAN_HEADLESS
	Cygnet::GLSL_PRELUDE = "#version 150\n";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	state->window.reset(SDL_CreateWindow(
		"SWAN Launcher  -  " SWAN_VERSION,
		450, 380, (
			SDL_WINDOW_OPENGL |
			SDL_WINDOW_RESIZABLE |
			SDL_WINDOW_HIDDEN |
			SDL_WINDOW_HIGH_PIXEL_DENSITY)));
	if (!state->window) {
		Swan::panic << "Failed to create window: " << SDL_GetError();
		return SDL_APP_FAILURE;
	}

	state->glContext = SDL_GL_CreateContext(state->window.get());
	if (!state->glContext) {
		Swan::panic << "Failed to create GL context: " << SDL_GetError();
		return SDL_APP_FAILURE;
	}
#ifdef __MINGW32__
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		Swan::panic << "GLAD failed to load GL!";
		return SDL_APP_FAILURE;
	}
#endif

 	SDL_GL_SetSwapInterval(1);

	SDL_SetWindowPosition(state->window.get(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_SetWindowMinimumSize(state->window.get(), 450, 300);
	SDL_ShowWindow(state->window.get());

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	Cygnet::glCheck();

	// Create one global VAO, so we can pretend VAOs don't exist
	glGenVertexArrays(1, &state->globalVao);
	glBindVertexArray(state->globalVao);
	Cygnet::glCheck();
#endif

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	state->imguiIo = ImGui::GetIO();
	state->imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	state->imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	state->imguiIo.IniFilename = nullptr;

#ifndef SWAN_HEADLESS
	ImGui_ImplSDL3_InitForOpenGL(state->window.get(), state->glContext);
	ImGui_ImplOpenGL3_Init("#version 150");
#endif

	// Create the game and mod list
	if (multiplayer.host != "") {
		Swan::info << "Connecting to multiplayer host: " << multiplayer.host << ":" << multiplayer.port;
		auto ptr = std::make_unique<Swan::MPGame>(compileMods, mods);
		ptr->connect(std::move(multiplayer));
		state->game = std::move(ptr);
	} else {
		if (!worldPath) {
			Swan::panic << "Missing world path!";
			return SDL_APP_FAILURE;
		}

		auto ptr = std::make_unique<Swan::Game>(compileMods, mods);

		// Load or create world
		if (std::filesystem::exists(worldPath)) {
			ptr->loadWorld(worldPath);
		} else {
			uint32_t seed;
			if (seedArg) {
				seed = *seedArg;
			} else {
				std::random_device dev;
				static_assert(
					sizeof(dev()) >= sizeof(uint32_t),
					"Maybe we need to generate the seed in a more fancy way?");
				seed = dev();
			}

			Swan::info << "Creating world with seed: " << seed;
			ptr->createWorld(worldPath, "core::default", seed);
		}

		state->game = std::move(ptr);
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	auto state = (AppState *)appstate;
	ImGui_ImplSDL3_ProcessEvent(event);

	switch (event->type) {
	case SDL_EVENT_QUIT:
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		return SDL_APP_SUCCESS;

	case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		state->framebufferSizeDirty = true;
		return SDL_APP_CONTINUE;

	case SDL_EVENT_WINDOW_RESIZED:
		state->framebufferSizeDirty = true;
		return SDL_APP_CONTINUE;

	case SDL_EVENT_KEY_DOWN:
		if (!state->imguiIo.WantCaptureKeyboard && !event->key.repeat) {
			state->game->inputs().onKeyDown(event->key.scancode);
		}
		return SDL_APP_CONTINUE;

	case SDL_EVENT_KEY_UP:
		if (!state->imguiIo.WantCaptureKeyboard) {
			state->game->inputs().onKeyUp(event->key.scancode);
		}
		return SDL_APP_CONTINUE;

	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		if (!state->imguiIo.WantCaptureMouse) {
			Swan::info << "BUTTON: " << int(event->button.button);
			state->game->inputs().onMouseDown(event->button.button);
		}
		return SDL_APP_CONTINUE;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		if (!state->imguiIo.WantCaptureMouse) {
			state->game->inputs().onMouseUp(event->button.button);
		}
		return SDL_APP_CONTINUE;

	case SDL_EVENT_MOUSE_MOTION:
		if (!state->imguiIo.WantCaptureMouse) {
			state->game->onMouseMove(
				event->motion.x * state->pixelRatio,
				event->motion.y * state->pixelRatio);
		}
		return SDL_APP_CONTINUE;

	case SDL_EVENT_MOUSE_WHEEL:
		if (!state->imguiIo.WantCaptureMouse) {
			state->game->onScrollWheel(event->wheel.y);
		}
		return SDL_APP_CONTINUE;

	default:
		return SDL_APP_CONTINUE;
	}
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	auto state = (AppState *)appstate;

	if (state->framebufferSizeDirty) {
		onFramebufferSizeChanged(state);
		state->framebufferSizeDirty = false;
	}

#ifndef SWAN_HEADLESS
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
#endif
	ImGui::NewFrame();

	// TODO
	float dt = 1.0 / 60;
	state->game->update(dt);

#ifndef SWAN_HEADLESS
	state->game->draw();
	Cygnet::glCheck();
	state->game->render();
	Cygnet::glCheck();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(state->window.get());
#endif

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	auto state = (AppState *)appstate;

	if (state->thumbnailPath) {
		state->game->screenshot(state->thumbnailPath, 256, 256);
	}
	state->game->onQuit();

#ifndef SWAN_HEADLESS
	glDeleteVertexArrays(1, &state->globalVao);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	SDL_GL_DestroyContext(state->glContext);
#endif
	ImGui::DestroyContext();

	delete state;
	NET_Quit();
	SDL_Quit();

	// Sometimes, destructing stuff hangs forever.
	// Especially AudioOutputUnitStop on macOS sometimes hangs
	// when destructing the SoundPlayer.
	// Since we've already saved the game, this doesn't really matter,
	// so let's just abort the process if we detect
	// teardown taking too long.
	std::thread([] {
		sleep(5);
		Swan::warn << "Haven't successfully exited in 5 seconds, exiting.";
		exit(0);
	}).detach();
}
