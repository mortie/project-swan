#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_sdl3.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <swan/log.h>
#include <swan/util.h>
#include <fstream>
#include <cygnet/gl.h>

#include "MainWindow.h"
#include "stylesheet.h"

struct AppState {
	ImGuiIO imguiIo;
	float pixelRatio = 1;
	MainWindow mainWindow;
	Swan::CPtr<SDL_Window, SDL_DestroyWindow> window;
	SDL_GLContext glContext;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		Swan::panic << "Failed to create window: " << SDL_GetError();
		return SDL_APP_FAILURE;
	}

	auto state = new AppState();
	*appstate = (void *)state;

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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	state->imguiIo = ImGui::GetIO();
	state->imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	state->imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	state->imguiIo.IniFilename = nullptr;

	StyleColors();
	ImGui::GetStyle().ScaleAllSizes(state->pixelRatio);
	ImGui::GetStyle().FontScaleDpi = state->pixelRatio;
	state->imguiIo.Fonts->AddFontFromFileTTF(
		"assets/NotoSans-Regular.ttf", 17 * state->pixelRatio);

	ImGui_ImplSDL3_InitForOpenGL(state->window.get(), state->glContext);
	ImGui_ImplOpenGL3_Init("#version 150");

	state->mainWindow.init();
	int width, height;
	SDL_GetWindowSize(state->window.get(), &width, &height);
	state->mainWindow.setSize(width, height);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	auto state = (AppState *)appstate;
	ImGui_ImplSDL3_ProcessEvent(event);

	switch (event->type) {
	case SDL_EVENT_QUIT:
	case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		Swan::info << "Quit event";
		return SDL_APP_SUCCESS;

	case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		state->pixelRatio = SDL_GetWindowDisplayScale(state->window.get());
		Swan::info << "Display scale: " << state->pixelRatio;
		state->imguiIo.Fonts->AddFontFromFileTTF(
			"assets/NotoSans-Regular.ttf", 17 * state->pixelRatio);
		state->imguiIo.Fonts->Build();
		return SDL_APP_CONTINUE;

	case SDL_EVENT_WINDOW_RESIZED:
		state->mainWindow.setSize(event->window.data1, event->window.data2);
		return SDL_APP_CONTINUE;

	default:
		return SDL_APP_CONTINUE;
	}
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	auto state = (AppState *)appstate;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	state->mainWindow.update();

	ImGui::Render();
	auto &io = ImGui::GetIO();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(state->window.get());

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	auto state = (AppState *)appstate;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DestroyContext(state->glContext);

	delete state;
	SDL_Quit();
}
