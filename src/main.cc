#include <time.h>
#include <unistd.h>
#include <vector>
#include <memory>
#include <chrono>
#include <ratio>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <SDL_image.h>
#include <string.h>
#include <imgui.h>
#include <cygnet/gl.h>
#include <cygnet/Renderer.h>

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

static Game *gameptr;
static double pixelRatio = 1;

static void keyCallback(GLFWwindow *, int key, int scancode, int action, int) {
	if (action == GLFW_PRESS) {
		gameptr->onKeyDown(scancode);
	} else if (action == GLFW_RELEASE) {
		gameptr->onKeyUp(scancode);
	}
}

static void mouseButtonCallback(GLFWwindow *, int button, int action, int) {
	if (action == GLFW_PRESS) {
		gameptr->onMouseDown(button);
	} else if (action == GLFW_RELEASE) {
		gameptr->onMouseUp(button);
	}
}

static void cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
	gameptr->onMouseMove(xpos * pixelRatio, ypos * pixelRatio);
}

static void scrollCallback(GLFWwindow *, double dx, double dy) {
	gameptr->onScrollWheel(dy);
}

static void windowSizeCallback(GLFWwindow *window, int width, int height) {
	int dw, dh;
	glfwGetFramebufferSize(window, &dw, &dh);
	glViewport(0, 0, dw, dh);
	Cygnet::glCheck();
	gameptr->cam_.size = {dw, dh};
	pixelRatio = (double)dw / (double)width;
}

int main(int argc, char **argv) {
	glfwInit();
	Deferred<glfwTerminate> glfw;

	glfwSetErrorCallback(+[](int error, const char* description) {
		warn << "GLFW Error: " << error << ": " << description;
	});

	int imgFlags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgFlags) == imgFlags, "Could not initialize SDL_Image");
	Deferred<IMG_Quit> sdlImage;

#ifdef __APPLE__
	Cygnet::GLSL_PRELUDE = "#version 150\n";
#else 
	Cygnet::GLSL_PRELUDE = "#version 320 es\nprecision mediump float;\n";
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow *window = glfwCreateWindow(640, 480, "Project: SWAN", nullptr, nullptr);
	if (!window) {
		panic << "Failed to create window";
		return 1;
	}

	glfwMakeContextCurrent(window);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Create one global VAO, so we can pretend VAOs don't exist
	GLuint globalVao;
	glGenVertexArrays(1, &globalVao);
	glBindVertexArray(globalVao);

	// Load and display application icon
	/*
	CPtr<SDL_Surface, SDL_FreeSurface> icon(
		IMG_Load("assets/icon.png"));
	sdlassert(icon, "Could not load icon");
	SDL_SetWindowIcon(window.sdlWindow(), icon.get());*/

	// Init ImGUI and ImGUI_SDL
	/*
	IMGUI_CHECKVERSION();
	CPtr<ImGuiContext, ImGui::DestroyContext> context(
		ImGui::CreateContext());

	ImGuiSDL::Initialize(renderer.get(), (int)win.getPixSize().x, (int)win.getPixSize().y);
	Deferred<ImGuiSDL::Deinitialize> imguiSDL;
	info << "Initialized with window size " << win.getPixSize();

	// ImGuiIO is to glue SDL and ImGUI together
	ImGuiIO& imguiIO = ImGui::GetIO();
	imguiIO.BackendPlatformName = "imgui_sdl + Project: SWAN";
	TODO */

	// Create a world
	Game game;
	std::vector<std::string> mods{ "core.mod" };
	game.createWorld("core::default", mods);

	gameptr = &game;
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetWindowSizeCallback(window, windowSizeCallback);

	// Initialize window size stuff
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		windowSizeCallback(window, width, height);
	}

	// Enable vsync
	glfwSwapInterval(1);

	auto prevTime = std::chrono::steady_clock::now();

	float fpsAcc = 0;
	float tickAcc = 0;

	int fCount = 0;
	int slowFrames = 0;
	while (!glfwWindowShouldClose(window)) {
		ZoneScopedN("game loop");

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
		if (dt <= 1 / 25.0) {
			ZoneScopedN("game update");
			game.update(dt);

		// Complex case: run multiple steps this iteration
		} else {
			int count = (int)ceil(dt / (1/30.0));
			float delta = dt / (float)count;

			// Don't be too noisy with the occasional double update
			if (count > 2) {
				info << "Delta time " << dt << "s. Running " << count
					<< " updates in one frame, with a delta as if we had "
					<< 1.0 / delta << " FPS.";
			}
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
			game.tick(1.0 / TICK_RATE);
		}

		{
			Cygnet::Color color = game.backgroundColor();
			glClearColor(color.r, color.g, color.b, color.a);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		// ImGUI
		//imguiIO.DeltaTime = dt;
		//ImGui::NewFrame();

		{
			ZoneScopedN("game draw");
			game.draw();
		}

		// Render ImGUI
		{
			ZoneScopedN("imgui render");
			//ImGui::Render();
			//ImGuiSDL::Render(ImGui::GetDrawData());
		}

		{
			ZoneScopedN("render present");
			glfwSwapBuffers(window);
		}

		glfwPollEvents();
		FrameMark
	}
}
