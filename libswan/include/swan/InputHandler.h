#pragma once

#include "Action.h"

#include <memory>

namespace Swan {

class Game;

class InputHandler {
public:
	InputHandler();
	~InputHandler();

	Action action(std::string_view name);
	void onKeyDown(int scancode);
	void onKeyUp(int scancode);
	void onMouseDown(int button);
	void onMouseUp(int button);
	void onButtonDown(int button);
	void onButtonUp(int button);

	void drawDebug();

protected:
	void setActions(std::vector<ActionSpec> actions);
	void beginFrame();
	void endFrame();

private:
	struct LogEntry;
	struct Gamepad;
	struct Impl;

	void updateGamepad(Gamepad &gamepad);

	void registerInput(std::string_view input, ActionKind kind, float *activation);
	void registerAxisInput(std::string_view input, float *activation);

	std::unique_ptr<Impl> impl_;

	friend Game;
};

}
