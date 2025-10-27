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
	Timestamp lastPlayedTimeStamp;
};

std::vector<World> listWorlds();
std::string makeWorld(std::string name);
std::string worldPath(std::string id);
std::string thumbnailPath(std::string id);
bool worldExists(std::string id);
void deleteWorld(std::string id);
void renameWorld(std::string id, std::string newName);
void updateWorldLastPlayedTime(std::string id);
