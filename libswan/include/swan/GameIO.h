#pragma once

#include <string_view>

namespace Swan {

class InputHandler;

class GameIO {
public:
	virtual ~GameIO() = default;

	virtual InputHandler &inputs() = 0;
	virtual void onMouseMove(float x, float y) = 0;
	virtual void onScrollWheel(float dy) = 0;
	virtual void onViewportSize(int w, int h) = 0;

	virtual void update(float dt) = 0;
	virtual void draw() = 0;
	virtual void render() = 0;
	virtual void screenshot(const char *path, int w, int h) = 0;
	virtual void onQuit() = 0;

	bool shouldQuit_ = false;
	float fpsLimit_ = 0;
	float fixedDeltaTime_ = 0;
	float timeScale_ = 1;
	bool vsync_ = false;
};

}
