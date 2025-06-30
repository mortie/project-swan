#pragma once

#include <vector>
#include <string>

struct World {
	std::string id;
	std::string name;
	std::string creationTime;
};

std::vector<World> listWorlds();
std::string makeWorld(std::string name);
std::string worldPath(std::string id);
bool worldExists(std::string id);
void deleteWorld(std::string id);
void renameWorld(std::string id, std::string newName);
