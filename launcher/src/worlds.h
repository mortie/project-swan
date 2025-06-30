#pragma once

#include <vector>
#include <string>

std::vector<std::string> listWorlds();
void makeWorldDir();
std::string worldPath(std::string name);
bool worldExists(std::string name);
void deleteWorld(std::string name);
void renameWorld(std::string oldName, std::string newName);
