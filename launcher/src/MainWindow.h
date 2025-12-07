#pragma once

#include "worlds.h"
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <optional>

class MainWindow {
public:
	MainWindow():
		running_(std::make_shared<std::atomic<bool>>(false))
	{
		loadWorlds();
	}

	void update();

	void setSize(int width, int height)
	{
		width_ = width;
		height_ = height;
	}

private:
	void loadWorlds();
	void launch(std::string worldID, std::optional<uint32_t> seed);

	int width_ = 0;
	int height_ = 0;
	std::vector<World> worlds_;
	std::string newWorldName_;
	std::string newWorldSeed_;
	std::string worldRenameBuffer_;

	std::shared_ptr<std::atomic<bool>> running_;
	bool wasRunning_ = false;
};
