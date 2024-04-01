#include "Game.h"

#include <fstream>
#include <math.h>
#include <time.h>
#include <memory>
#include <imgui/imgui.h>

#include "traits/InventoryTrait.h"
#include "EntityCollectionImpl.h"

namespace Swan {

void Game::createWorld(
	const std::string &worldgen, const std::vector<std::string> &modPaths)
{
	world_ = std::make_unique<World>(this, time(NULL), modPaths);

	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	world_->setWorldGen(worldgen);
	world_->setCurrentPlane(world_->addPlane());
	world_->spawnPlayer();
}

void Game::loadWorld(
	std::istream &is, const std::vector<std::string> &modPaths)
{
	world_ = std::make_unique<World>(this, time(NULL), modPaths);

	for (auto &mod: world_->mods_) {
		mod.mod_->start(*world_);
	}

	MsgStream::Parser parser(is);
	world_->deserialize(parser);
}

Vec2 Game::getMousePos()
{
	return (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;
}

TilePos Game::getMouseTile()
{
	auto pos = (getMouseScreenPos() * 2 - renderer_.winScale()) / cam_.zoom + cam_.pos;

	return TilePos{(int)floor(pos.x), (int)floor(pos.y)};
}

Cygnet::Color Game::backgroundColor()
{
	return world_->backgroundColor();
}

void Game::draw()
{
	if (debugShowMenu_) {
		ImGui::Begin("Debug Menu", &debugShowMenu_, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Draw collision boxes", &debugDrawCollisionBoxes_);
		ImGui::Checkbox("Draw chunk boundaries", &debugDrawChunkBoundaries_);

		bool prevEnableVSync = debugEnableVSync_;
		ImGui::Checkbox("Enable VSync", &debugEnableVSync_);
		if (debugEnableVSync_ && !prevEnableVSync) {
			glfwSwapInterval(1);
		}
		else if (!debugEnableVSync_ && prevEnableVSync) {
			glfwSwapInterval(0);
		}

		if (ImGui::Button("Save")) {
			std::fstream f("world.mp", std::ios_base::out);
			if (f) {
				info << "Serializing to world.mp...";
				MsgStream::Serializer w(f);
				world_->serialize(w);
				info << "Done.";
			} else {
				warn << "Failed to open world.mp!";
			}
		}

		ImGui::SliderFloat(
			"Time scale", &debugTimeScale_, 0, 3.0, "%.03f",
			ImGuiSliderFlags_Logarithmic);
		if (ImGui::BeginPopupContextItem("Time scale menu")) {
			if (ImGui::MenuItem("Reset")) {
				debugTimeScale_ = 1.0;
			}
			ImGui::EndPopup();
		}

		float oldVolume = soundPlayer_.volume();
		float volume = oldVolume;
		ImGui::SliderFloat(
			"Volume", &volume, 0, 1.0, "%.03f",
			ImGuiSliderFlags_Logarithmic);
		if (ImGui::BeginPopupContextItem("Volume menu")) {
			if (ImGui::MenuItem("Reset")) {
				volume = 1.0;
			}
			ImGui::EndPopup();
		}

		if (volume != oldVolume) {
			soundPlayer_.volume(volume);
		}

		auto &tile = world_->currentPlane().getTile(getMouseTile());
		ImGui::Text("Tile: %s\n", tile.name.c_str());

		ImGui::Text("Give Item:");
		ImGui::BeginChild("Give Item", {0, 200});
		for (auto &[name, item]: world_->items_) {
			if (item.hidden) {
				continue;
			}

			if (ImGui::Button(name.c_str())) {
				auto *inventory = world_->playerRef_.trait<InventoryTrait>();
				info << "Giving player " << name;
				ItemStack stack(&item, 1);
				inventory->insert(stack);
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	world_->draw(renderer_);

	world_->ui();
}

void Game::update(float dt)
{
	dt *= debugTimeScale_;

	// Zoom the window using the scroll wheel
	cam_.zoom += (float)wasWheelScrolled() * 0.05f * cam_.zoom;
	if (cam_.zoom > 1) {
		cam_.zoom = 1;
	}
	else if (cam_.zoom < 0.025) {
		cam_.zoom = 0.025;
	}

	if (wasLiteralKeyPressed(GLFW_KEY_F3)) {
		debugShowMenu_ = !debugShowMenu_;
	}

	world_->update(dt);

	didScroll_ = 0;
	didPressKeys_.reset();
	didReleaseKeys_.reset();
	didPressLiteralKeys_.reset();
	didReleaseLiteralKeys_.reset();
	didPressButtons_.reset();
	didReleaseButtons_.reset();
}

void Game::tick(float dt)
{
	dt *= debugTimeScale_;
	world_->tick(dt);
}

}
