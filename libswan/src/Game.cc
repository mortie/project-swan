#include "Game.h"

#include <fstream>
#include <math.h>
#include <time.h>
#include <memory>
#include <filesystem>
#include <imgui/imgui.h>

#include "traits/InventoryTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

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

	sbon::Reader r(&is);
	world_->deserialize(r);
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

		bool prevEnableVSync = enableVSync_;
		ImGui::Checkbox("Enable VSync", &enableVSync_);
		if (enableVSync_ && !prevEnableVSync) {
			glfwSwapInterval(1);
		}
		else if (!enableVSync_ && prevEnableVSync) {
			glfwSwapInterval(0);
		}

		if (ImGui::Button("Save")) {
			save();
		}

		ImGui::SliderFloat(
			"Time scale", &timeScale_, 0, 3.0, "%.03f",
			ImGuiSliderFlags_Logarithmic);
		if (ImGui::BeginPopupContextItem("Time scale menu")) {
			if (ImGui::MenuItem("Reset")) {
				timeScale_ = 1.0;
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

		ImGui::SliderFloat(
			"UI scale", &uiCam_.zoom, 0, 1.0, "%.03f",
			ImGuiSliderFlags_Logarithmic);
		if (ImGui::BeginPopupContextItem("UI scale menu")) {
			if (ImGui::MenuItem("Reset")) {
				uiCam_.zoom = 1.0/16;
			}
			ImGui::EndPopup();
		}

		ImGui::SliderFloat(
			"FPS limit", &fpsLimit_, 10, 360.0, "%.03f", 0);
		if (ImGui::BeginPopupContextItem("FPS limit menu")) {
			if (ImGui::MenuItem("Disable")) {
				fpsLimit_ = 0;
			}
			else if (ImGui::MenuItem("30")) {
				fpsLimit_ = 30;
			}
			else if (ImGui::MenuItem("60")) {
				fpsLimit_ = 60;
			} else if (ImGui::MenuItem("90")) {
				fpsLimit_ = 90;
			} else if (ImGui::MenuItem("120")) {
				fpsLimit_ = 120;
			} else if (ImGui::MenuItem("144")) {
				fpsLimit_ = 144;
			}
			ImGui::EndPopup();
		}

		if (fpsLimit_ < 10) {
			fpsLimit_ = 0;
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
}

void Game::update(float dt)
{
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
	world_->tick(dt);
}

void Game::save()
{
	std::fstream f("world.sb.new", std::ios_base::out);

	if (f) {
		info << "Serializing to world.sb.new...";
		sbon::Writer w(&f);
		world_->serialize(w);
		info << "Renaming to world.sb...";
		std::filesystem::rename("world.sb.new", "world.sb");
		info << "Done.";
	}
	else {
		warn << "Failed to open world.sb.new!";
	}
}

}
