#pragma once

#include "worlds.h"
#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <imgui/imgui.h>

class GLTexture {
public:
	GLTexture() = default;
	GLTexture(GLTexture &&);
	~GLTexture();

	GLTexture &operator=(GLTexture &&other)
	{
		this->~GLTexture();
		new (this) GLTexture(std::move(other));
		return *this;
	}

	static std::optional<GLTexture> fromFile(const char *path);

	ImTextureID id() { return texture_; }
	int width() { return width_; }
	int height() { return height_; }


private:
	int32_t texture_ = -1;
	int width_ = 0;
	int height_ = 0;
};

class MainWindow {
public:
	MainWindow():
		running_(std::make_shared<std::atomic<bool>>(false))
	{}

	void init() { loadWorlds(); }
	void update();

	void setSize(int width, int height)
	{
		width_ = width;
		height_ = height;
	}

private:
	struct WorldWrapper {
		World world;
		std::string prettyCreationTime;
		std::string prettyLastPlayedTime;
		GLTexture texture;
	};

	void loadWorlds();
	void launch(std::string worldID, std::optional<uint32_t> seed);
	std::string getNewWorldName();

	int width_ = 0;
	int height_ = 0;
	std::vector<WorldWrapper> worlds_;
	std::string newWorldName_;
	std::string newWorldSeed_;
	std::string worldRenameBuffer_;

	std::shared_ptr<std::atomic<bool>> running_;
	bool wasRunning_ = false;
	bool reloadWorlds_ = false;
};
