#pragma once

#include <swan-common/Vector2.h>
#include <memory>

namespace Cygnet {

struct WindowState;

class Window {
public:
	Window(const char *name, int w, int h);
	~Window();

	void makeCurrent();
	void clear();
	void flip();
	void onResize(int w, int h);
	SwanCommon::Vec2i size() { return { w_, h_ }; }

private:
	std::unique_ptr<WindowState> state_;
	int w_;
	int h_;
};

}
