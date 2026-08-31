#pragma once

#include <functional>
#include <cygnet/Renderer.h>
#include <memory>

#include "Action.h"
#include "EntityCollection.h"
#include "GameIO.h"
#include "InputHandler.h"
#include "MPClient.h"
#include "Mod.h"
#include "WorldData.h"
#include "WorldPlane.h"
#include "swan/HashMap.h"

namespace Swan {

class MPGame final: public GameIO {
public:
	MPGame(std::function<bool()> recompileMods, HashMap<ModInfo> mods);

	InputHandler &inputs() override { return inputHandler_; }
	void onMouseMove(float x, float y) override;
	void onScrollWheel(float dy) override;
	void onViewportSize(int w, int h) override;

	Vec2 getMousePos() override;
	TilePos getMouseTile() override;
	Vec2 uiPosFromWorldPos(Vec2 worldPos) override
	{ return (worldPos / uiCam_.zoom) * cam_.zoom; }

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
	void onMessageFromServer(mp_proto::ServerToClient::Reader &r);
	void initInputHandler();
	void sendPlayerState();

	Cygnet::RenderCamera cam_{.zoom = 1.0 / 8};
	Cygnet::RenderCamera uiCam_{.zoom = 1.0 / 16};

	std::function<bool()> recompileMods_;
	HashMap<ModInfo> mods_;

	InputHandler inputHandler_;
	MPClient client_;
	float tickTimer_ = 0;
	std::unique_ptr<WorldData> data_;
	std::unique_ptr<WorldPlane> plane_;

	EntityRef player_;
};

}
