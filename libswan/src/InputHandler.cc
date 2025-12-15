#include <bitset>
#include <cassert>
#include <cmath>
#include <vector>
#include <array>
#include <imgui/imgui.h>
#include <deque>
#include <optional>

#include <swan/HashMap.h>
#include <swan/log.h>

#include "inputmaps/axes.h"
#include "inputmaps/buttons.h"
#include "inputmaps/keys.h"
#include "inputmaps/mouse.h"
#include "InputHandler.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Swan {

struct InputHandler::LogEntry {
	const char *kind;
	const char *name;
	int value;
};

struct InputHandler::Gamepad {
	GLFWgamepadstate state = {};
	GLFWgamepadstate prevState = {};
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

	std::bitset<GLFW_KEY_LAST> pressedKeys;
	std::bitset<GLFW_MOUSE_BUTTON_LAST> pressedMouseButtons;

	std::array<std::optional<Gamepad>, GLFW_JOYSTICK_LAST + 1>
		gamepads;
	std::bitset<GLFW_JOYSTICK_LAST + 1> disabledGamepads;

	bool verbose = false;
	std::deque<LogEntry> log;
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
	bool ignore =
		scancode < 0 ||
		size_t(scancode) >= impl_->pressedKeys.size() ||
		impl_->pressedKeys[scancode];
	if (ignore) {
		warn << "Ignoring unknown key: " << scancode;
		return;
	}

	impl_->pressedKeys[scancode] = true;

	if (impl_->verbose) {
		impl_->log.push_front(LogEntry{
			.kind = "KEY DOWN",
			.name = keyboardKeyToName(scancode).data(),
			.value = scancode,
		});
	}

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
	bool ignore =
		scancode < 0 ||
		size_t(scancode) >= impl_->pressedKeys.size() ||
		!impl_->pressedKeys[scancode];
	if (ignore) {
		return;
	}

	impl_->pressedKeys[scancode] = false;

	if (impl_->verbose) {
		impl_->log.push_front(LogEntry{
			.kind = "KEY UP",
			.name = keyboardKeyToName(scancode).data(),
			.value = scancode,
		});
	}

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
	bool ignore =
		button < 0 ||
		size_t(button) >= impl_->pressedMouseButtons.size() ||
		impl_->pressedMouseButtons[button];
	if (ignore) {
		warn << "Ignoring unknown mouse button: " << button;
		return;
	}

	impl_->pressedMouseButtons[button] = true;

	if (impl_->verbose) {
		impl_->log.push_front(LogEntry{
			.kind = "MOUSE DOWN",
			.name = mouseButtonToName(button).data(),
			.value = button,
		});
	}

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
	bool ignore =
		button < 0 ||
		size_t(button) >= impl_->pressedMouseButtons.size() ||
		!impl_->pressedMouseButtons[button];
	if (ignore) {
		return;
	}

	impl_->pressedMouseButtons[button] = false;

	if (impl_->verbose) {
		impl_->log.push_front(LogEntry{
			.kind = "MOUSE UP",
			.name = mouseButtonToName(button).data(),
			.value = button,
		});
	}

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

void InputHandler::drawDebug()
{
	impl_->verbose = true;
	int gamepadCount = 0;
	for (auto &pad: impl_->gamepads) {
		if (pad) {
			gamepadCount += 1;
		}
	}

	ImGui::Text("Connected gamepads: %d", gamepadCount);
	for (size_t jid = 0; jid < impl_->gamepads.size(); ++jid) {
		if (!impl_->gamepads[jid]) {
			continue;
		}

		auto &pad = *impl_->gamepads[jid];
		ImGui::Text("Gamepad %zu (%s):", jid, glfwGetGamepadName(jid));

		for (size_t ax = 0; ax <= GLFW_GAMEPAD_AXIS_LAST; ++ax) {
			const char *name = gamepadAxisToName(ax).data();
			float val = pad.state.axes[ax];
			ImGui::Text("* Axis %zu (%s): %f", ax, name, val);
		}

		for (size_t btn = 0; btn < GLFW_GAMEPAD_BUTTON_LAST; ++btn) {
			const char *name = gamepadButtonToName(btn).data();
			int val = pad.state.buttons[btn];
			ImGui::Text("* %zu (%s): %d", btn, name, val);
		}
	}

	if (!impl_->log.empty()) {
		ImGui::Text("Log:");
		for (auto &entry: impl_->log) {
			ImGui::Text(
				"* Button %s: %d (%s)",
				entry.kind, entry.value, entry.name);
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
				if (impl_->disabledGamepads[jid]) {
					continue;
				}

				if (!glfwJoystickIsGamepad(jid)) {
					impl_->disabledGamepads[jid] = true;
					continue;
				}

				impl_->gamepads[jid].emplace();
			}

			Gamepad &gamepad = *impl_->gamepads[jid];

			glfwGetGamepadState(jid, &gamepad.state);
			updateGamepad(gamepad);
		} else if (impl_->gamepads[jid]) {
			Gamepad &gamepad = *impl_->gamepads[jid];
			gamepad.state = {};
			updateGamepad(gamepad);

			impl_->gamepads[jid] = std::nullopt;
			impl_->disabledGamepads[jid] = false;
		}
	}
}

void InputHandler::endFrame()
{
	for (auto action: impl_->oneshotActionsActivatedThisFrame) {
		action->activation = 0;
	}
	impl_->oneshotActionsActivatedThisFrame.clear();

	if (impl_->verbose) {
		impl_->verbose = false;
		if (impl_->log.size() > 16) {
			impl_->log.pop_back();
		}
	} else if (!impl_->log.empty()) {
		info << "Clearing log";
		std::deque<LogEntry> empty;
		impl_->log.swap(empty);
	}
}

void InputHandler::updateGamepad(Gamepad &gamepad)
{
	for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; ++i) {
		float val = gamepad.state.axes[i];
		if (std::abs(val) < 0.1) {
			val = 0;
		}

		float delta = val - gamepad.prevState.axes[i];
		if (delta == 0) {
			continue;
		}
		gamepad.prevState.axes[i] = val;

		auto it = impl_->joystickAxes.find(i);
		if (it == impl_->joystickAxes.end()) {
			return;
		}

		for (auto &w: it->second) {
			w.action->activation += delta * w.multiplier;
			if (std::abs(w.action->activation) < 0.01) {
				w.action->activation = 0;
			}
		}
	}

	for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
		bool pressed = gamepad.state.buttons[i];
		bool prevPressed = gamepad.prevState.buttons[i];
		if (pressed == prevPressed) {
			continue;
		}
		gamepad.prevState.buttons[i] = pressed;

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
	int code = -1;
	if (category == "key") {
		code = keyboardKeyFromName(name);
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		code = mouseButtonFromName(name);
		map = &impl_->mouseButtons;
	}
	else if (category == "button") {
		code = gamepadButtonFromName(name);
		map = &impl_->gamepadButtons;
	}
	else if (category == "axis") {
		code = gamepadAxisFromName(name);
		map = &impl_->joystickAxes;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	if (code < 0) {
		warn << "Unknown " << category << ": " << name;
		return;
	}

	(*map)[code].push_back({
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

		auto code = gamepadAxisFromName(secondary);
		if (code < 0) {
			warn << "Unknown axis: " << secondary;
			return;
		}

		impl_->joystickAxes[code].push_back({
			.action = action,
			.kind = ActionKind::AXIS,
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
	int (*codeFunc)(std::string_view) = nullptr;
	if (category == "key") {
		codeFunc = keyboardKeyFromName;
		map = &impl_->keys;
	}
	else if (category == "mouse") {
		codeFunc = mouseButtonFromName;
		map = &impl_->mouseButtons;
	}
	else if (category == "button") {
		codeFunc = gamepadButtonFromName;
		map = &impl_->gamepadButtons;
	}
	else {
		warn << "Unknown category in input: " << input;
		return;
	}

	int neg = codeFunc(negative);
	if (neg < 0) {
		warn << "Unknown " << category << ": " << negative;
		return;
	}

	int pos = codeFunc(positive);
	if (pos < 0) {
		warn << "Unknown " << category << ": " << positive;
		return;
	}

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
