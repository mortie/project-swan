#include "MPGame.h"

#include <imgui/imgui.h>
#include <swan/constants.h>
#include <swan/log.h>

namespace Swan {

static constexpr float TICK_DELTA = 1.0 / TICK_RATE;

void MPGame::update(float dt)
{
	tickTimer_ += dt;
	if (tickTimer_ >= TICK_DELTA) {
		tickTimer_ -= TICK_DELTA;
		tick(TICK_DELTA);
	}
}

void MPGame::draw()
{
	ImGui::Begin(
		"Connection Status", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

	const char *stateStr = "<unknown>";
	auto state = client_.state();
	switch (state.tag) {
	case MPClient::CONNECTING:
		stateStr = "Connecting...";
		break;
	case MPClient::HANDSHAKING:
		stateStr = "Handshaking...";
		break;
	case MPClient::CONNECTED:
		stateStr = "Connected.";
		break;
	case MPClient::CLOSED:
		stateStr = "Closed.";
		break;
	case MPClient::KICKED:
		stateStr = "Kicked.";
		break;
	case MPClient::SHUTDOWN:
		stateStr = "Server shut down.";
		break;
	case MPClient::ERROR:
		stateStr = "Error.";
		break;
	}

	ImGui::Text("Connection state: %s", stateStr);
	if (!state.reason.empty()) {
		ImGui::Text("Reason: %s", state.reason.c_str());
	}

	renderer_.clear();

	ImGui::End();
}

void MPGame::render()
{
	renderer_.render(cam_);
}

void MPGame::onQuit()
{
	info << "Sending quit message";
	client_.end();
}

void MPGame::connect(MPClient::Options opts)
{
	opts.requestWorld = true;
	client_.connect(std::move(opts));
}

void MPGame::tick(float dt)
{
	client_.tick(dt);

	mp_proto::ServerToClient::Reader r;
	while (client_.receive(r)) {
		if (r.isInitialSync()) {
			info << "Got initial sync message yay";
		}
	}
}

}
