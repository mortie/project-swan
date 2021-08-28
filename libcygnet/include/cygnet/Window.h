#pragma once

#include <swan-common/Vector2.h>
#include <memory>

#include "util.h"

struct SDL_Window;

namespace Cygnet {

struct WindowState;

class Window {
public:
	Window(const char *name, int w, int h);
	~Window();

	void makeCurrent();
	void clear(Color color = {});
	void flip();
	void onResize(int w, int h);
	SwanCommon::Vec2i size() { return size_; }
	double pixelRatio() { return ratio_; }

	SDL_Window *sdlWindow();

private:
	std::unique_ptr<WindowState> state_;
	SwanCommon::Vec2i size_;
	double ratio_;
};

}
