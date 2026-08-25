#pragma once

#include <functional>
#include <cygnet/Renderer.h>
#include <memory>

#include "GameIO.h"
#include "InputHandler.h"
#include "MPClient.h"
#include "Mod.h"
#include "WorldPlane.h"
#include "swan/HashMap.h"

namespace Swan {

class MPGame: public GameIO {
public:
	MPGame(std::function<bool()> recompileMods, HashMap<ModInfo> mods):
		recompileMods_(recompileMods),
		mods_(std::move(mods))
	{}

	InputHandler &inputs() override { return inputs_; }
	void onMouseMove(float x, float y) override {}
	void onScrollWheel(float dy) override {}
	void onViewportSize(int w, int h) override {}

	void update(float dt) override;
	void draw() override;
	void render() override;
	void screenshot(const char *path, int w, int h) override {}
	void onQuit() override;

	void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center) override
	{}

	void playSound(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center,
		SoundHandle &handle) override
	{}

	void connect(MPClient::Options opts);

private:
	void tick(float dt);

	Cygnet::Renderer renderer_;
	Cygnet::RenderCamera cam_{.zoom = 1.0 / 8};

	std::function<bool()> recompileMods_;
	HashMap<ModInfo> mods_;

	InputHandler inputs_;
	MPClient client_;
	float tickTimer_ = 0;
	std::unique_ptr<WorldPlane> plane_;
};

}
