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
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
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
static ImGuiIO *imguiIo;
static double pixelRatio = 1;

static void keyCallback(GLFWwindow *, int key, int scancode, int action, int) {
	if (imguiIo->WantCaptureKeyboard) {
		return;
	}

	if (action == GLFW_PRESS) {
		gameptr->onKeyDown(scancode, key);
	} else if (action == GLFW_RELEASE) {
		gameptr->onKeyUp(scancode, key);
	}
}

static void mouseButtonCallback(GLFWwindow *, int button, int action, int) {
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	if (action == GLFW_PRESS) {
		gameptr->onMouseDown(button);
	} else if (action == GLFW_RELEASE) {
		gameptr->onMouseUp(button);
	}
}

static void cursorPositionCallback(GLFWwindow *, double xpos, double ypos) {
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	gameptr->onMouseMove(xpos * pixelRatio, ypos * pixelRatio);
}

static void scrollCallback(GLFWwindow *, double dx, double dy) {
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	gameptr->onScrollWheel(dy);
}

static void windowSizeCallback(GLFWwindow *window, int width, int height) {
	int dw, dh;
	glfwGetFramebufferSize(window, &dw, &dh);
	glViewport(0, 0, dw, dh);
	Cygnet::glCheck();
	gameptr->cam_.size = {dw, dh};
	double newPixelRatio = (double)dw / (double)width;

	if (newPixelRatio != pixelRatio) {
		pixelRatio = newPixelRatio;
		imguiIo->FontGlobalScale = 1.0 / pixelRatio;
		imguiIo->Fonts->ClearFonts();

		struct ImFontConfig config;
		config.SizePixels = 13 * pixelRatio;
		imguiIo->Fonts->AddFontDefault(&config);
	}
}

int main(int argc, char **argv) {
	glfwSetErrorCallback(+[](int error, const char* description) {
		warn << "GLFW Error: " << error << ": " << description;
	});

	if (!glfwInit()) {
		panic << "Initializing GLFW failed.";
		return 1;
	}
	defer(glfwTerminate());

	int imgFlags = IMG_INIT_PNG;
	imgassert(IMG_Init(imgFlags) == imgFlags, "Could not initialize SDL_Image");
	defer(IMG_Quit());

	Cygnet::GLSL_PRELUDE = "#version 150\n";
	const char *imguiGlsl = Cygnet::GLSL_PRELUDE;

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

	// Enable vsync
	glfwSwapInterval(1);

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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	defer(ImGui::DestroyContext());

	imguiIo = &ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	defer(ImGui_ImplGlfw_Shutdown());
	ImGui_ImplOpenGL3_Init(imguiGlsl);
	defer(ImGui_ImplOpenGL3_Shutdown());

	// Create one global VAO, so we can pretend VAOs don't exist
	GLuint globalVao;
	glGenVertexArrays(1, &globalVao);
	glBindVertexArray(globalVao);

	// Initialize window size stuff
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		windowSizeCallback(window, width, height);
	}

	auto prevTime = std::chrono::steady_clock::now();

	float fpsAcc = 0;
	float tickAcc = 0;

	int fCount = 0;
	int slowFrames = 0;
	while (!glfwWindowShouldClose(window)) {
		ZoneScopedN("game loop");

		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

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
			ZoneScopedN("game draw");
			game.draw();
		}

		{
			ZoneScopedN("imgui draw");
			ImGui::Render();
		}

		{
			Cygnet::Color color = game.backgroundColor();
			glClearColor(color.r, color.g, color.b, color.a);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		{
			ZoneScopedN("game render");
			game.render();
		}

		{
			ZoneScopedN("imgui render");
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		{
			ZoneScopedN("render present");
			glfwSwapBuffers(window);
		}

		FrameMark;
	}
}
