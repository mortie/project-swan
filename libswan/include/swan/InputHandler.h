#pragma once

#include "Action.h"
#include "swan/HashMap.h"

#include <unordered_map>

namespace Swan {

class Game;

class InputHandler {
public:
	InputHandler();
	~InputHandler();

	Action *action(std::string_view name);
	void onKeyDown(int scancode);
	void onKeyUp(int scancode);
	void onMouseDown(int button);
	void onMouseUp(int button);

protected:
	struct Impl;

	void setActions(std::vector<ActionSpec> actions);
	void endFrame();
	void registerInput(std::string_view input, ActionKind kind, Action *action);
	void registerAxisInput(std::string_view input, Action *action);

	std::unique_ptr<Impl> impl_;

	friend Game;
};

}
