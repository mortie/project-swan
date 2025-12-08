#include <cstdlib>
#include <random>
#include <sstream>
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
#include <filesystem>

#include <swan/swan.h>
#include <swan/assets.h>

#include "build.h"

#define HAS_MODERN_GLFW \
	GLFW_VERSION_MAJOR > 3 || \
	(GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4)

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
		gameptr->inputs().onKeyDown(key);
	}
	else if (action == GLFW_RELEASE) {
		gameptr->inputs().onKeyUp(key);
	}
}

static void mouseButtonCallback(GLFWwindow *, int button, int action, int)
{
	if (imguiIo->WantCaptureMouse) {
		return;
	}

	if (action == GLFW_PRESS) {
		gameptr->inputs().onMouseDown(button);
	}
	else if (action == GLFW_RELEASE) {
		gameptr->inputs().onMouseUp(button);
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
	std::optional<uint32_t> seedArg;
	const char *worldPath = nullptr;
	backward::SignalHandling sh;
	std::vector<std::string> mods;
	const char *swanRoot = ".";
	bool doCompileMods = true;
	const char *thumbnailPath = nullptr;
	for (int i = 1; i < argc; ++i) {
		std::string_view arg = argv[i];
		if (arg == "--mod") {
			i += 1;
			mods.push_back(argv[i]);
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
			warn << "Unexpected option: " << arg;
		}
	}

	if (mods.empty()) {
		panic << "Empty mods list!";
	}

	if (!worldPath) {
		panic << "Missing world path!";
	}

	auto compileMods = [&]() {
		if (!doCompileMods) {
			return true;
		}

		for (auto &mod: mods) {
			if (!SwanBuild::build(mod.c_str(), swanRoot)) {
				return false;
			}
		}

		return true;
	};
	if (!compileMods()) {
		return 1;
	}

	glfwSetErrorCallback(+[] (int error, const char *description) {
		warn << "GLFW Error: " << error << ": " << description;
	});

	if (!glfwInit()) {
		panic << "Initializing GLFW failed.";
		return 1;
	}
	SWAN_DEFER(glfwTerminate());

	{ // Load custom input mappings from file
		std::fstream f("assets/gamecontrollerdb.txt");
		if (f) {
			std::stringstream ss;
			ss << f.rdbuf();
			auto str = std::move(ss).str();
			glfwUpdateGamepadMappings(str.c_str());
		} else {
			Swan::warn << "Failed to open assets/gamecontrollerdb.txt";
		}
	}

	Cygnet::GLSL_PRELUDE = "#version 150\n";

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow *window = glfwCreateWindow(
		640, 480, "Project: SWAN  -  " SWAN_VERSION,
		nullptr, nullptr);
	if (!window) {
		panic << "Failed to create window";
		return 1;
	}

	glfwMakeContextCurrent(window);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	Cygnet::glCheck();

	// Create the game and mod list
	Game game(compileMods);

	// Load or create world
	if (std::filesystem::exists(worldPath)) {
		game.loadWorld(worldPath, mods);
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

		info << "Creating world with seed: " << seed;
		game.createWorld(worldPath, "core::default", seed, mods);
	}

	gameptr = &game;
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSwapInterval(1);
	Cygnet::glCheck();
	game.enableVSync_ = true;

	GLFWmonitor *currentMonitor = [&] {
		int winX, winY, winW, winH;
		glfwGetWindowPos(window, &winX, &winY);
#if HAS_MODERN_GLFW
		if (glfwGetError(nullptr) == GLFW_FEATURE_UNAVAILABLE) {
			return glfwGetPrimaryMonitor();
		}
#endif

		glfwGetWindowSize(window, &winW, &winH);
		int centerX = winX + (winW / 2);
		int centerY = winY + (winH / 2);

		int n;
		GLFWmonitor **monitors = glfwGetMonitors(&n);
		for (int i = 0; i < n; ++i) {
			int monX, monY, monW, monH;
			glfwGetMonitorWorkarea(monitors[i], &monX, &monY, &monW, &monH);
			if (centerX < monX || centerX > monX + monW) {
				continue;
			}
			if (centerY < monY || centerY > monY + monH) {
				continue;
			}

			Swan::info << "Found monitor: " << glfwGetMonitorName(monitors[i]);
			return monitors[i];
		}

		Swan::info << "Found no monitor, using primary";
		return glfwGetPrimaryMonitor();
	}();

	const GLFWvidmode *mode = glfwGetVideoMode(currentMonitor);
	game.fpsLimit_ = mode->refreshRate;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	SWAN_DEFER(ImGui::DestroyContext());

	imguiIo = &ImGui::GetIO();
	imguiIo->IniFilename = nullptr;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	SWAN_DEFER(ImGui_ImplGlfw_Shutdown());
	ImGui_ImplOpenGL3_Init("#version 150");
	SWAN_DEFER(ImGui_ImplOpenGL3_Shutdown());

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
		if (game.fpsLimit_ > 0) {
			std::chrono::duration<float> minDur(1.0 / game.fpsLimit_);
			if (dur < minDur) {
				using T = std::chrono::steady_clock::duration;
				auto sleepTime = std::chrono::duration_cast<T>(minDur - dur);
				std::this_thread::sleep_for(sleepTime);
				now += sleepTime;
				dur = now - prevTime;
			}
		}

		prevTime = now;
		float dt = dur.count();

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

		// If the game has a fixed delta time,
		// ignore anything we've measured so far.
		if (game.fixedDeltaTime_) {
			dt = *game.fixedDeltaTime_;
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

	if (thumbnailPath) {
		game.screenshot(thumbnailPath, 256, 256);
	}

	game.save();

	// Sometimes, destructing stuff hangs forever.
	// Especially AudioOutputUnitStop on macOS sometimes hangs
	// when destructing the SoundPlayer.
	// Since we've already saved the game, this doesn't really matter,
	// so let's just abort the process if we detect
	// teardown taking too long.
	std::thread([] {
		sleep(5);
		warn << "Haven't successfully exited in 5 seconds, exiting.";
		exit(0);
	}).detach();

	return 0;
}
