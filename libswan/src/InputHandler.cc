#include <bitset>
#include <cassert>
#include <cmath>
#include <unordered_set>
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

struct InputHandler::Gamepad {
	std::array<float, 6> prevAxes;
	std::array<float, 6> axes;
	std::bitset<32> prevButtons;
	std::bitset<32> buttons;
};

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

	std::array<std::optional<Gamepad>, GLFW_JOYSTICK_LAST + 1> gamepads;
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
		if (w.kind == ActionKind::ONESHOT) {
			impl_->oneshotActionsActivatedThisFrame.push_back(w.action);
		}
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

void InputHandler::onButtonDown(int button)
{
	auto it = impl_->gamepadButtons.find(button);
	if (it == impl_->gamepadButtons.end()) {
		return;
	}

	for (auto &w: it->second) {
		w.action->activation += w.multiplier;
		if (w.kind == ActionKind::ONESHOT) {
			impl_->oneshotActionsActivatedThisFrame.push_back(w.action);
		}
	}
}

void InputHandler::onButtonUp(int button)
{
	auto it = impl_->gamepadButtons.find(button);
	if (it == impl_->gamepadButtons.end()) {
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

	// Fill vector
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

void InputHandler::beginFrame()
{
	for (int jid = 0; jid <= GLFW_JOYSTICK_LAST; ++jid) {
		bool present = glfwJoystickPresent(jid);
		if (present) {
			if (!impl_->gamepads[jid]) {
				impl_->gamepads[jid].emplace();
			}

			Gamepad &gamepad = *impl_->gamepads[jid];

			int count;
			const float *axes = glfwGetJoystickAxes(jid, &count);
			count = std::min(size_t(count), gamepad.axes.size());
			for (int i = 0; i < count; ++i) {
				gamepad.axes[i] = axes[i];
			}

			const auto *buttons = glfwGetJoystickButtons(jid, &count);
			count = std::min(size_t(count), gamepad.buttons.size());
			for (int i = 0; i < count; ++i) {
				gamepad.buttons[i] = buttons[i];
			}

			updateGamepad(gamepad);
		} else if (impl_->gamepads[jid]) {
			Gamepad &gamepad = *impl_->gamepads[jid];
			gamepad.axes = {};
			gamepad.buttons = {};
			updateGamepad(gamepad);

			impl_->gamepads[jid] = std::nullopt;
		}
	}
}

void InputHandler::endFrame()
{
	for (auto action: impl_->oneshotActionsActivatedThisFrame) {
		action->activation = 0;
	}
	impl_->oneshotActionsActivatedThisFrame.clear();
}

void InputHandler::updateGamepad(Gamepad &gamepad)
{
	for (int i = 0; i < int(gamepad.axes.size()); ++i) {
		float val = gamepad.axes[i];
		if (std::abs(val) < 0.1) {
			val = 0;
		}

		float delta = val - gamepad.prevAxes[i];
		if (delta == 0) {
			continue;
		}
		gamepad.prevAxes[i] = val;

		auto it = impl_->joystickAxes.find(i);
		if (it == impl_->keys.end()) {
			return;
		}

		for (auto &w: it->second) {
			w.action->activation += delta * w.multiplier;
			if (std::abs(w.action->activation) < 0.01) {
				w.action->activation = 0;
			}
		}
	}

	for (int i = 0; i < int(gamepad.buttons.size()); ++i) {
		bool pressed = gamepad.buttons[i];
		bool prevPressed = gamepad.prevButtons[i];
		if (pressed == prevPressed) {
			continue;
		}
		gamepad.prevButtons[i] = pressed;

		if (pressed) {
			onButtonDown(i);
		} else {
			onButtonUp(i);
		}
	}
}

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

		auto it = gamepadAxisFromName.find(secondary);
		if (it == gamepadAxisFromName.end()) {
			warn << "Unknown axis: " << secondary;
			return;
		}

		impl_->joystickAxes[it->second].push_back({
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
	else if (category == "button") {
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
