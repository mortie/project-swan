#include <cassert>
#include <cmath>
#include <vector>

#include <swan/HashMap.h>

#include "inputmaps/axes.h"
#include "inputmaps/buttons.h"
#include "inputmaps/keys.h"
#include "inputmaps/mouse.h"
#include "InputHandler.h"
#include "log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Swan {

struct InputHandler::Impl {
	std::vector<Action> actions;
	HashMap<Action *> actionsByName;
	Action dummyAction;

	std::vector<Action *> oneshotActionsActivatedThisFrame;

	struct ActionWrapper {
		Action *action;
		ActionKind kind;
		float multiplier = 1;
	};

	std::unordered_map<int, std::vector<ActionWrapper>> keys;
	std::unordered_map<int, std::vector<ActionWrapper>> mouseButtons;
	std::unordered_map<int, std::vector<ActionWrapper>> gamepadButtons;
	std::unordered_map<int, std::vector<ActionWrapper>> joystickAxes;
};

InputHandler::InputHandler(): impl_(std::make_unique<Impl>()) {}
InputHandler::~InputHandler() = default;

Action *InputHandler::action(std::string_view name)
{
	auto it = impl_->actionsByName.find(name);
	if (it == impl_->actionsByName.end()) {
		warn << "Unknown action: " << name;
		return &impl_->dummyAction;
	}

	return it->second;
}

void InputHandler::onKeyDown(int scancode)
{
	auto it = impl_->keys.find(scancode);
	if (it == impl_->keys.end()) {
		return;
	}

	for (auto &w: it->second) {
		w.action->activation += w.multiplier;
		if (w.kind == ActionKind::ONESHOT) {
			impl_->oneshotActionsActivatedThisFrame.push_back(w.action);
		}
	}
}

void InputHandler::onKeyUp(int scancode)
{
	auto it = impl_->keys.find(scancode);
	if (it == impl_->keys.end()) {
		return;
	}

	for (auto &w: it->second) {
		if (w.kind == ActionKind::ONESHOT) {
			continue;
		}

		w.action->activation -= w.multiplier;
		if (std::abs(w.action->activation) < 0.01) {
			w.action->activation = 0;
		}
	}
}

void InputHandler::onMouseDown(int button)
{
	auto it = impl_->mouseButtons.find(button);
	if (it == impl_->mouseButtons.end()) {
		return;
	}

	for (auto &w: it->second) {
		w.action->activation += w.multiplier;
	}
}

void InputHandler::onMouseUp(int button)
{
	auto it = impl_->mouseButtons.find(button);
	if (it == impl_->mouseButtons.end()) {
		return;
	}

	for (auto &w: it->second) {
		if (w.kind == ActionKind::ONESHOT) {
			continue;
		}

		w.action->activation -= w.multiplier;
		if (std::abs(w.action->activation) < 0.01) {
			w.action->activation = 0;
		}
	}
}

void InputHandler::setActions(std::vector<ActionSpec> actions)
{
	impl_ = std::make_unique<Impl>();

	// Fill vectors
	impl_->actions.resize(actions.size());

	// Fill actionsByName hash map
	auto it = impl_->actions.begin();
	for (auto &spec: actions) {
		impl_->actionsByName[spec.name] = &*it;
		it++;
	}

	// Fill inputs
	it = impl_->actions.begin();
	for (auto &spec: actions) {
		Action *action = &*it;
		it++;

		for (auto &input: spec.defaultInputs) {
			registerInput(input, spec.kind, action);
		}
	}
}

void InputHandler::endFrame()
{
	for (auto action: impl_->oneshotActionsActivatedThisFrame) {
		action->activation = 0;
	}
}

static int lookupFromName(const HashMap<int> &map, std::string_view name)
{
	auto it = map.find(name);
	if (it == map.end()) {
		return -1;
	}
	return it->second;
};

void InputHandler::registerInput(
	std::string_view input,
	ActionKind kind,
	Action *action)
{
	if (kind == ActionKind::AXIS) {
		registerAxisInput(input, action);
		return;
	}

	auto colon = input.find(':');
	if (colon == std::string::npos) {
		warn << "Invalid input: " << input;
		return;
	}

	auto category = input.substr(0, colon);
	auto name = input.substr(colon + 1);

	std::unordered_map<int, std::vector<Impl::ActionWrapper>> *map;
	const HashMap<int> *nameMap;
	if (category == "key") {
		nameMap = &scanCodeFromName;
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		nameMap = &mouseButtonFromName;
		map = &impl_->mouseButtons;
	}
	else if (category == "button") {
		nameMap = &gamepadButtonFromName;
		map = &impl_->gamepadButtons;
	}
	else if (category == "axis") {
		nameMap = &gamepadAxisFromName;
		map = &impl_->joystickAxes;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	auto it = nameMap->find(name);
	if (it == nameMap->end()) {
		warn << "Unknown " << category << ": " << name;
		return;
	}
	int id = it->second;

	(*map)[id].push_back({
		.action = action,
		.kind = kind,
	});
}

void InputHandler::registerAxisInput(std::string_view input, Action *action)
{
	auto colon = input.find(':');
	if (colon == std::string::npos) {
		warn << "Invalid input: " << input;
		return;
	}

	auto category = input.substr(0, colon);
	auto secondary = input.substr(colon + 1);

	if (category == "axis") {
		float multiplier = 1;
		if (secondary.starts_with('-')) {
			multiplier = -1;
			secondary = secondary.substr(1);
		}

		int id = lookupFromName(gamepadAxisFromName, secondary);
		if (id < 0) {
			warn << "Unknown axis: " << secondary;
			return;
		}

		impl_->joystickAxes[id].push_back({
			.action = action,
			.multiplier = multiplier,
		});
		return;
	}

	auto semicolon = secondary.find(';');
	if (semicolon == std::string::npos) {
		warn << "Axis actions need two inputs, got one: " << input;
		return;
	}

	auto negative = secondary.substr(0, semicolon);
	auto positive = secondary.substr(semicolon + 1);

	std::unordered_map<int, std::vector<Impl::ActionWrapper>> *map;
	const HashMap<int> *nameMap;
	if (category == "key") {
		nameMap = &scanCodeFromName;
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		nameMap = &mouseButtonFromName;
		map = &impl_->mouseButtons;
	}
	else if (category == "btn") {
		nameMap = &gamepadButtonFromName;
		map = &impl_->gamepadButtons;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	auto negIt = nameMap->find(negative);
	if (negIt == nameMap->end()) {
		warn << "Unknown " << category << ": " << negative;
		return;
	}
	int neg = negIt->second;

	auto posIt = nameMap->find(positive);
	if (posIt == nameMap->end()) {
		warn << "Unknown " << category << ": " << positive;
		return;
	}
	int pos = posIt->second;

	(*map)[neg].push_back({
		.action = action,
		.kind = ActionKind::AXIS,
		.multiplier = -1,
	});
	(*map)[pos].push_back({
		.action = action,
		.kind = ActionKind::AXIS,
		.multiplier = 1
	});
}

}
