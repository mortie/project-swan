#pragma once

#include <chrono>
#include <vector>
#include <string>

using Timestamp = std::chrono::time_point<
	std::chrono::system_clock,
	std::chrono::duration<long long, std::ratio<1, 1>>>;

struct World {
	std::string id;
	std::string name;
	std::string creationTime;
	std::string lastPlayedTime;
	time_t lastPlayedTimestamp;
};

std::vector<World> listWorlds();
std::string makeWorld(std::string name);
std::string worldPath(std::string_view id);
std::string thumbnailPath(std::string_view id);
bool worldExists(std::string_view id);
void deleteWorld(std::string_view id);
void renameWorld(std::string_view id, std::string newName);
void updateWorldLastPlayedTime(std::string_view id);
