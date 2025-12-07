#include "MainWindow.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <string_view>
#include <thread>
#include <swan/log.h>

#include "worlds.h"
#include "system.h"

using namespace std::chrono_literals;

static std::string getNewWorldName(std::span<World> worlds)
{
	constexpr const char *NAME = "Default";
	std::string name = NAME;
	bool collision = false;
	for (auto &world: worlds) {
		if (world.name == name) {
			collision = true;
			break;
		}
	}

	if (!collision) {
		return name;
	}

	int num = 2;
	while (true) {
		name = NAME;
		name += " ";
		name += std::to_string(num);

		bool collision = false;
		for (auto &world: worlds) {
			if (world.name == name) {
				collision = true;
				break;
			}
		}

		if (!collision) {
			return name;
		}

		num += 1;
	}
}

static std::optional<uint32_t> calcSeed(std::string_view seedStr)
{
	std::optional<uint32_t> seed;
	uint32_t seedNum = 0;
	for (char ch: seedStr) {
		if (ch <= 32) {
			continue;
		}

		seedNum *= 10;
		seedNum += (unsigned char)ch - '0';
		seed = seedNum;
	}

	return seed;
}

void MainWindow::update()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_Always);
	ImGui::Begin(
		"Main Window", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoTitleBar);

	bool running = running_->load();
	if (running) {
		ImGui::BeginDisabled();
	}

	if (!running && wasRunning_) {
		loadWorlds();
	}

	wasRunning_ = running;
	bool reloadWorlds = false;

	for (auto &world: worlds_) {
		ImGui::PushID(world.id.c_str());

		ImGui::Text("%s", world.name.c_str());
		ImGui::Text("Created:     %s", world.creationTime.c_str());
		ImGui::Text("Last played: %s", world.lastPlayedTime.c_str());

		if (ImGui::Button("Delete")) {
			ImGui::OpenPopup("Delete");
		}

		ImGui::SameLine();

		if (ImGui::Button("Rename")) {
			worldRenameBuffer_ = world.name;
			ImGui::OpenPopup("Rename");
		}

		ImGui::SameLine();

		if (ImGui::Button("Play")) {
			launch(world.id, std::nullopt);
		}

		if (ImGui::BeginPopupModal("Delete", nullptr, ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Delete %s?", world.name.c_str());

			if (ImGui::Button("No")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Yes")) {
				deleteWorld(world.id);
				reloadWorlds = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_NoResize)) {
			ImGui::PushItemWidth(200);
			ImGui::InputText("##New Name", &worldRenameBuffer_);
			ImGui::PopItemWidth();

			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Rename")) {
				renameWorld(world.id, worldRenameBuffer_);
				reloadWorlds = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::InputText("Name", &newWorldName_);
	ImGui::InputText("Seed", &newWorldSeed_);
	if (ImGui::Button("Create World")) {
		std::string id = makeWorld(newWorldName_);
		launch(id, calcSeed(newWorldSeed_));
	}

	if (running) {
		ImGui::EndDisabled();
	}

	if (reloadWorlds) {
		loadWorlds();
	}

	ImGui::End();

	// Throttle rendering when running
	if (running) {
		std::this_thread::sleep_for(100ms);
	}
}

void MainWindow::launch(
	std::string worldID,
	std::optional<uint32_t> seed)
{
	running_->store(true);
	updateWorldLastPlayedTime(worldID);
	std::thread([running = running_, worldID = std::move(worldID), seed] {
		std::string cmd = "./bin/swan";
		appendArg(cmd, "--mod");
		appendArg(cmd, "core.mod");
		appendArg(cmd, "--world");
		appendArg(cmd, worldPath(worldID));
		appendArg(cmd, "--thumbnail");
		appendArg(cmd, thumbnailPath(worldID));

		if (seed) {
			appendArg(cmd, "--seed");
			appendArg(cmd, std::to_string(*seed));
		}

		Swan::info << "Running command: " << cmd;
		runCommand(cmd.c_str());

		running->store(false);
	}).detach();
}

void MainWindow::loadWorlds()
{
	worlds_ = listWorlds();
	newWorldName_ = getNewWorldName(worlds_);
	newWorldSeed_ = "";
}
