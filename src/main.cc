#include <unistd.h>
#include <stdlib.h>
#include <vector>
#include <chrono>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <backward.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <cygnet/gl.h>
#include <cygnet/Renderer.h>

#include <swan/swan.h>
#include <swan/assets.h>

using namespace Swan;

#define errassert(expr, str, errfn) do { \
			if (!(expr)) { \
				panic << (str) << ": " << errfn(); \
				return EXIT_FAILURE; \
			} \
} while (0)

static Game *gameptr;
static ImGuiIO *imguiIo;
static double pixelRatio = 1;

static void keyCallback(GLFWwindow *, int key, int scancode, int action, int)
{
	if (imguiIo->WantCaptureKeyboard) {
		return;
	}

	if (action == GLFW_PRESS) {
		gameptr->onKeyDown(scancode, key);
	}
	else if (action == GLFW_RELEASE) {
		gameptr->onKeyUp(scancode, key);
	}
}

static void mouseButtonCallback(GLFWwindow *, int button, int action, int)
{
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	if (action == GLFW_PRESS) {
		gameptr->onMouseDown(button);
	}
	else if (action == GLFW_RELEASE) {
		gameptr->onMouseUp(button);
	}
}

static void cursorPositionCallback(GLFWwindow *, double xpos, double ypos)
{
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	gameptr->onMouseMove(xpos * pixelRatio, ypos * pixelRatio);
}

static void scrollCallback(GLFWwindow *, double dx, double dy)
{
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	gameptr->onScrollWheel(dy);
}

static void framebufferSizeCallback(GLFWwindow *window, int dw, int dh)
{
	int width, height;

	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, dw, dh);
	Cygnet::glCheck();
	gameptr->cam_.size = {dw, dh};
	gameptr->uiCam_.size = {dw, dh};
	double newPixelRatio = (double)dw / (double)width;

	if (newPixelRatio != pixelRatio) {
		pixelRatio = newPixelRatio;
		imguiIo->FontGlobalScale = 1.0 / pixelRatio;
		imguiIo->Fonts->Clear();

		struct ImFontConfig config;
		config.SizePixels = 13 * pixelRatio;
		imguiIo->Fonts->AddFontDefault(&config);
		imguiIo->Fonts->Build();
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}
}

int main(int argc, char **argv)
{
	backward::SignalHandling sh;

	char *swanRoot = getenv("SWAN_ROOT");

	if (swanRoot != nullptr && swanRoot[0] != '\0') {
		Swan::assetBasePath = swanRoot;
	}

	glfwSetErrorCallback(+[] (int error, const char *description) {
		warn << "GLFW Error: " << error << ": " << description;
	});

	if (!glfwInit()) {
		panic << "Initializing GLFW failed.";
		return 1;
	}
	defer(glfwTerminate());

	Cygnet::GLSL_PRELUDE = "#version 150\n";

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
	Cygnet::glCheck();

	// Enable vsync
	glfwSwapInterval(1);
	Cygnet::glCheck();

	// Create the game and mod list
	Game game;
	std::vector<std::string> mods{"core.mod"};

	// Load or create world
	std::ifstream worldFile("world.mp");
	if (worldFile) {
		game.loadWorld(worldFile, mods);
	}
	else {
		game.createWorld("core::default", mods);
	}

	gameptr = &game;
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	defer(ImGui::DestroyContext());

	imguiIo = &ImGui::GetIO();

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	defer(ImGui_ImplGlfw_Shutdown());
	ImGui_ImplOpenGL3_Init("#version 150");
	defer(ImGui_ImplOpenGL3_Shutdown());

	{
		int dw, dh;
		glfwGetFramebufferSize(window, &dw, &dh);
		framebufferSizeCallback(window, dw, dh);
	}

	// Create one global VAO, so we can pretend VAOs don't exist
	GLuint globalVao;
	glGenVertexArrays(1, &globalVao);
	glBindVertexArray(globalVao);

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
		Cygnet::glCheck();

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
			if (slowFrames == 0) {
				warn << "Delta time too high! (" << dt << "s)";
			}
			slowFrames += 1;

			// And we never want to do physics as if our one frame is greater than
			// 0.5 seconds.
			if (dt > 0.5) {
				dt = 0.5;
			}
		}
		else if (slowFrames > 0) {
			if (slowFrames > 1) {
				warn << slowFrames << " consecutive slow frames.";
			}
			slowFrames = 0;
		}

		// Scale delta time by time scale.
		// Everything after this will use a scaled delta time
		// rather than the real delta time.
		dt *= game.timeScale_;

		// Simple case: we can keep up, only need one physics update
		if (dt <= 1 / 25.0) {
			ZoneScopedN("game update");
			game.update(dt);

			// Complex case: run multiple steps this iteration
		}
		else {
			int count = (int)ceil(dt / (1 / 30.0));
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
			Cygnet::glCheck();
		}

		{
			ZoneScopedN("imgui draw");
			ImGui::Render();
			Cygnet::glCheck();
		}

		{
			Cygnet::Color color = game.backgroundColor();
			glClearColor(color.r, color.g, color.b, color.a);
			glClear(GL_COLOR_BUFFER_BIT);
			Cygnet::glCheck();
		}

		{
			ZoneScopedN("game render");
			game.render();
			Cygnet::glCheck();
		}

		{
			ZoneScopedN("imgui render");
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			Cygnet::glCheck();
		}

		{
			ZoneScopedN("render present");
			glfwSwapBuffers(window);
			Cygnet::glCheck();
		}

		FrameMark;
	}

	game.save();

	return 0;
}
