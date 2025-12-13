#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cygnet/gl.h>
#include <cygnet/util.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <swan/log.h>
#include <swan/util.h>
#include <fstream>

#include "MainWindow.h"
#include "stylesheet.h"

static ImGuiIO *imguiIo;
static double pixelRatio = 1;
static MainWindow mainWindow;

static void framebufferSizeCallback(GLFWwindow *window, int dw, int dh)
{
	int width, height;

	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, dw, dh);
	Cygnet::glCheck();
	double newPixelRatio = (double)dw / (double)width;

	if (newPixelRatio != pixelRatio) {
		pixelRatio = newPixelRatio;
		imguiIo->FontGlobalScale = 1.0 / pixelRatio;
		imguiIo->Fonts->Clear();

		imguiIo->Fonts->AddFontFromFileTTF(
			"assets/NotoSans-Regular.ttf", 17 * pixelRatio);
		imguiIo->Fonts->Build();
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}

	mainWindow.setSize(width, height);
}

int main()
{
	glfwSetErrorCallback(+[] (int error, const char *description) {
		Swan::warn << "GLFW Error: " << error << ": " << description;
	});

	if (!glfwInit()) {
		Swan::panic << "Initializing GLFW failed.";
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

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHintString(GLFW_WAYLAND_APP_ID, "coffee.mort.Swan");
	GLFWwindow *window = glfwCreateWindow(
		450, 380, "SWAN Launcher  -  " SWAN_VERSION,
		nullptr, nullptr);
	if (!window) {
		Swan::panic << "Failed to create window";
		return 1;
	}

	glfwSetWindowSizeLimits(window, 450, 300, GLFW_DONT_CARE, GLFW_DONT_CARE);
	glfwMakeContextCurrent(window);
#ifdef __MINGW32__
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		Swan::panic << "GLAD failed to load GL!";
		return 1;
	}
#endif

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSwapInterval(1);
	Cygnet::glCheck();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	SWAN_DEFER(ImGui::DestroyContext());
	Cygnet::glCheck();

	imguiIo = &ImGui::GetIO();
	imguiIo->IniFilename = nullptr;

	StyleColors();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	SWAN_DEFER(ImGui_ImplGlfw_Shutdown());
	ImGui_ImplOpenGL3_Init("#version 150");
	SWAN_DEFER(ImGui_ImplOpenGL3_Shutdown());
	Cygnet::glCheck();

	{
		int dw, dh;
		glfwGetFramebufferSize(window, &dw, &dh);
		framebufferSizeCallback(window, dw, dh);
	}

	mainWindow.init();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		Cygnet::glCheck();

		mainWindow.update();
		Cygnet::glCheck();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		Cygnet::glCheck();
	}
}
