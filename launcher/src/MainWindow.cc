#include "MainWindow.h"

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <string_view>
#include <thread>
#include <swan/log.h>
#include <cygnet/gl.h>
#include <span>
#include <stb/stb_image.h>

#include "cygnet/util.h"
#include "worlds.h"
#include "system.h"

using namespace std::chrono_literals;

GLTexture::GLTexture(GLTexture &&other)
{
	texture_ = other.texture_;
	width_ = other.width_;
	height_ = other.height_;
	other.texture_ = -1;
}

GLTexture::~GLTexture()
{
	if (texture_ >= 0) {
		GLuint tex = texture_;
		glDeleteTextures(1, &tex);
		Cygnet::glCheck();
	}
}

std::optional<GLTexture> GLTexture::fromFile(const char *path)
{
	GLTexture tex;

	unsigned char *data = stbi_load(
		path, &tex.width_, &tex.height_, nullptr, 4);
	if (!data) {
		Swan::warn << "Failed to load image: " << path;
		return std::nullopt;
	}

	GLuint texID;
	glGenTextures(1, &texID);
	tex.texture_ = texID;
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	Cygnet::glCheck();

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, tex.width_, tex.height_,
		0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	Cygnet::glCheck();
	stbi_image_free(data);

	return tex;
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

static GLTexture loadWorldThumbnail(std::string id)
{
	auto tex = GLTexture::fromFile(thumbnailPath(std::move(id)).c_str());
	if (tex) {
		return std::move(*tex);
	}

	tex = GLTexture::fromFile("assets/unknown-world.png");
	if (tex) {
		return std::move(*tex);
	}

	Swan::panic << "Failed to load unknown-world asset!";
	abort();
}

void MainWindow::update()
{
	bool running = running_->load();

	if (!running && wasRunning_) {
		reloadWorlds_ = true;
	}
	wasRunning_ = running;

	if (reloadWorlds_) {
		loadWorlds();
		reloadWorlds_ = false;
	}

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width_, height_), ImGuiCond_Always);
	ImGui::Begin(
		"Main Window", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoScrollbar);
	ImGui::SetScrollY(0);

	if (running) {
		ImGui::BeginDisabled();
	}

	auto avail = ImGui::GetContentRegionAvail();
	ImGui::BeginChild(
		"Worlds",
		ImVec2(avail.x,avail.y - 35),
		0, ImGuiWindowFlags_NoScrollbar);

	for (auto &wrapper: worlds_) {
		auto &world = wrapper.world;
		auto &tex = wrapper.texture;
		ImGui::PushID(world.id.c_str());

		int thumbSize = 105;
		auto cursor = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(
			cursor.x + ImGui::GetContentRegionAvail().x - thumbSize,
			cursor.y));
		ImGui::Image(tex.id(), ImVec2(thumbSize, thumbSize));
		ImGui::SetCursorPos(cursor);

		ImGui::Text("%s", world.name.c_str());

		ImGui::Text("Created:");
		ImGui::SameLine();
		ImGui::SetCursorPosX(90);
		ImGui::Text("%s", wrapper.prettyCreationTime.c_str());

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);

		ImGui::Text("Last played:");
		ImGui::SameLine();
		ImGui::SetCursorPosX(90);
		ImGui::Text("%s", wrapper.prettyLastPlayedTime.c_str());

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

		if (ImGui::Button("Delete", ImVec2(100, 0))) {
			ImGui::OpenPopup("Delete");
		}

		ImGui::SameLine(0, 4);

		if (ImGui::Button("Rename", ImVec2(100, 0))) {
			worldRenameBuffer_ = world.name;
			ImGui::OpenPopup("Rename");
		}

		ImGui::SameLine(0, 4);

		if (ImGui::Button("Play", ImVec2(100, 0))) {
			launch(world.id, std::nullopt);
		}

		if (ImGui::BeginPopupModal("Delete", nullptr, ImGuiWindowFlags_NoResize)) {
			ImGui::Text("Delete \"%s\"?", world.name.c_str());
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
			ImGui::Text("This cannot be undone.");

			if (ImGui::Button("No", ImVec2(70, 0))) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Yes", ImVec2(70, 0))) {
				deleteWorld(world.id);
				reloadWorlds_ = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopupModal("Rename", nullptr, ImGuiWindowFlags_NoResize)) {
			ImGui::PushItemWidth(200);
			ImGui::InputText("##New Name", &worldRenameBuffer_);
			ImGui::PopItemWidth();

			if (ImGui::Button("Cancel", ImVec2(94, 0))) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Rename", ImVec2(94, 0))) {
				renameWorld(world.id, worldRenameBuffer_);
				reloadWorlds_ = true;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		ImGui::Dummy(ImVec2(0, 0));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0, 0));

		ImGui::PopID();
	}

	ImGui::EndChild();

	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 180);
	ImGui::InputText("##Name", &newWorldName_);
	ImGui::SameLine(0, 5);
	ImGui::PushItemWidth(80);
	ImGui::InputTextWithHint("##Seed", "Seed", &newWorldSeed_);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("Create World")) {
		std::string id = makeWorld(newWorldName_);
		launch(id, calcSeed(newWorldSeed_));
	}

	if (running) {
		ImGui::EndDisabled();
	}

	ImGui::End();

	// Throttle rendering when running
	if (running) {
		std::this_thread::sleep_for(100ms);
	}
}

void MainWindow::loadWorlds()
{
	auto worlds = listWorlds();
	worlds_.clear();
	worlds_.reserve(worlds.size());
	for (auto &world: worlds) {
		std::string worldID = world.id;
		std::string prettyCreationTime =
			world.creationTime.substr(0, world.creationTime.find('+'));
		std::string prettyLastPlayedTime =
			world.lastPlayedTime.substr(0, world.lastPlayedTime.find('+'));
		worlds_.push_back({
			.world = std::move(world),
			.prettyCreationTime = std::move(prettyCreationTime),
			.prettyLastPlayedTime = std::move(prettyLastPlayedTime),
			.texture = loadWorldThumbnail(worldID),
		});
	}

	newWorldName_ = getNewWorldName();
	newWorldSeed_ = "";
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

std::string MainWindow::getNewWorldName()
{
	constexpr const char *NAME = "New World";
	std::string name = NAME;
	bool collision = false;
	for (auto &wrapper: worlds_) {
		if (wrapper.world.name == name) {
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
		for (auto &wrapper: worlds_) {
			if (wrapper.world.name == name) {
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
