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
	std::vector<Action> oneshots;
	std::vector<Action> continuous;
	std::vector<Action> axis;
	HashMap<Action *> actionsByName;
	Action dummyAction;

	struct ActionWrapper {
		Action *action = nullptr;
		bool negative = false;
	};

	std::unordered_map<int, ActionWrapper> keys;
	std::unordered_map<int, ActionWrapper> mouseButtons;
	std::unordered_map<int, ActionWrapper> gamepadButtons;
	std::unordered_map<int, ActionWrapper> joystickAxes;
};

InputHandler::InputHandler(): impl_(std::make_unique<Impl>()) {}
InputHandler::~InputHandler() = default;

const Action &InputHandler::getAction(std::string_view name)
{
	auto it = impl_->actionsByName.find(name);
	if (it == impl_->actionsByName.end()) {
		warn << "Unknown action: " << name;
		return impl_->dummyAction;
	}

	return *it->second;
}

void InputHandler::setActions(std::vector<ActionSpec> actions)
{
	impl_->oneshots.clear();
	impl_->continuous.clear();
	impl_->actionsByName.clear();
	impl_->keys.clear();
	impl_->mouseButtons.clear();
	impl_->gamepadButtons.clear();
	impl_->joystickAxes.clear();

	// Fill vectors
	for (auto &spec: actions) {
		switch (spec.kind) {
		case ActionKind::CONTINUOUS:
			impl_->continuous.push_back({});
			break;
		case ActionKind::ONESHOT:
			impl_->oneshots.push_back({});
			break;
		case ActionKind::AXIS:
			impl_->axis.push_back({});
			break;
		}
	}

	// Fill actionsByName hash map
	auto continuousIt = impl_->continuous.begin();
	auto oneshotIt = impl_->continuous.begin();
	auto axisIt = impl_->axis.begin();
	for (auto &spec: actions) {
		switch (spec.kind) {
		case ActionKind::CONTINUOUS:
			impl_->actionsByName[spec.name] = &*continuousIt;
			continuousIt++;
			break;
		case ActionKind::ONESHOT:
			impl_->actionsByName[spec.name] = &*oneshotIt;
			oneshotIt++;
			break;
		case ActionKind::AXIS:
			impl_->actionsByName[spec.name] = &*axisIt;
			axisIt++;
			break;
		}
	}

	// Fill inputs
	continuousIt = impl_->continuous.begin();
	oneshotIt = impl_->continuous.begin();
	axisIt = impl_->axis.begin();
	for (auto &spec: actions) {
		Action *action = nullptr;
		switch (spec.kind) {
		case ActionKind::CONTINUOUS:
			action = &*continuousIt;
			continuousIt++;
			break;
		case ActionKind::ONESHOT:
			action = &*oneshotIt;
			oneshotIt++;
			break;
		case ActionKind::AXIS:
			action = &*axisIt;
			axisIt++;
			break;
		}
		assert(action);

		for (auto &input: spec.defaultInputs) {
			if (spec.kind == ActionKind::AXIS) {
				registerAxisInput(input, action);
			} else {
				registerInput(input, action);
			}
		}
	}
}

void InputHandler::endFrame()
{
	for (auto &action: impl_->oneshots) {
		action.activation = NAN;
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

void InputHandler::registerInput(std::string_view input, Action *action)
{
	auto colon = input.find(':');
	if (colon == std::string::npos) {
		warn << "Invalid input: " << input;
		return;
	}

	auto category = input.substr(0, colon);
	auto name = input.substr(colon + 1);

	int id;
	std::unordered_map<int, Impl::ActionWrapper> *map;
	if (category == "key") {
		id = lookupFromName(scanCodeFromName, name);
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		id = lookupFromName(mouseButtonFromName, name);
		map = &impl_->mouseButtons;
	}
	else if (category == "button") {
		id = lookupFromName(gamepadButtonFromName, name);
		map = &impl_->gamepadButtons;
	}
	else if (category == "axis") {
		id = lookupFromName(gamepadAxisFromName, name);
		map = &impl_->joystickAxes;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	if (map->contains(id)) {
		warn << "Duplicate input: " << input;
		return;
	}

	(*map)[id] = {
		.action = action,
		.negative = false,
	};
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
		bool negative = false;
		if (secondary.starts_with('-')) {
			negative = true;
			secondary = secondary.substr(1);
		}

		int id = lookupFromName(gamepadAxisFromName, secondary);
		if (id < 0) {
			warn << "Unknown axis: " << secondary;
			return;
		}

		impl_->joystickAxes[id] = {
			.action = action,
			.negative = negative,
		};
		return;
	}

	auto semicolon = secondary.find(';');
	if (semicolon == std::string::npos) {
		warn << "Axis actions need two inputs, got one: " << input;
		return;
	}

	auto negative = secondary.substr(0, semicolon);
	auto positive = secondary.substr(semicolon + 1);

	int neg, pos;
	std::unordered_map<int, Impl::ActionWrapper> *map;
	if (category == "key") {
		neg = scanCodeFromName(negative);
		pos = scanCodeFromName(positive);
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		neg = mouseButtonFromName(negative);
		pos = mouseButtonFromName(positive);
		map = &impl_->mouseButtons;
	}
	else if (category == "btn") {
		neg = gamepadButtonFromName(negative);
		pos = gamepadButtonFromName(positive);
		map = &impl_->gamepadButtons;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	if (neg < 0 || pos < 0) {
		warn << "Invalid input: " << input;
		return;
	}

	if (map->contains(neg) || map->contains(pos)) {
		warn << "Duplicate input: " << input;
		return;
	}

	(*map)[neg] = {
		.action = action,
		.negative = true,
	};
	(*map)[pos] = {
		.action = action,
		.negative = false,
	};
}

}
