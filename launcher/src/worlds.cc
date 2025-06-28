#include "worlds.h"

#include <filesystem>

std::vector<std::string> listWorlds()
{
	if (!std::filesystem::exists("worlds")) {
		return {};
	}

	std::vector<std::filesystem::directory_entry> entries;

	for (auto &ent: std::filesystem::directory_iterator("worlds")) {
		entries.push_back(ent);
	}

	std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b) {
		return a.last_write_time() > b.last_write_time();
	});

	std::vector<std::string> names;
	for (auto &ent: entries) {
		names.push_back(ent.path().stem());
	}

	return names;
}

void makeWorldDir()
{
	std::filesystem::create_directories("worlds");
}

std::string worldPath(std::string name)
{
	return "worlds/" + name + ".swan";
}

bool worldExists(std::string name)
{
	return std::filesystem::exists(worldPath(name));
}
