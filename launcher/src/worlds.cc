#include "worlds.h"

#include <cstdlib>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cpptoml.h>

#include <date/date.h>
#include <date/tz.h>
#include <sstream>

void randomID(std::string &str)
{
	constexpr const char *alphabet = "0123456789abcdef";
	str.resize(16);
	std::generate_n(str.begin(), str.size(), [] { return alphabet[rand() % 16]; });
}

std::vector<World> listWorlds()
{
	if (!std::filesystem::exists("worlds")) {
		return {};
	}

	std::vector<World> worlds;

	for (auto &ent: std::filesystem::directory_iterator("worlds")) {
		std::string id = ent.path().filename();
		std::string tomlPath = "worlds/" + id + "/world.toml";

		auto table = cpptoml::parse_file(tomlPath);
		auto name = table->get_as<std::string>("name");
		auto creationTime = table->get_as<std::string>("creation-time");
		auto lastPlayedTime = table->get_as<std::string>("last-played-time");

		date::local_time<std::chrono::seconds> lastPlayedLocalTime;
		Timestamp lastPlayedTimeStamp;
		if (lastPlayedTime) {
			// This is pretty much the worst API I have ever had to deal with T_T
			// Parsing straight into a time_point works on macOS,
			// but not on Linux (guessing it works on libc++ but not libstdc++).
			// However, parsing into a local_time seems to work.
			// If only the documentation suggested what type it expects...
			std::istringstream str(*lastPlayedTime);
			std::chrono::minutes offset;
			str >> date::parse("%F %T%z", lastPlayedLocalTime, offset);
			lastPlayedTimeStamp = Timestamp((lastPlayedLocalTime - offset).time_since_epoch());
		}

		worlds.push_back(World {
			.id = id,
			.name = name.value_or("Untitled"),
			.creationTime = creationTime.value_or("Unknown"),
			.lastPlayedTime = lastPlayedTime.value_or("Unknown"),
			.lastPlayedTimeStamp = lastPlayedTimeStamp,
		});
	}

	std::sort(worlds.begin(), worlds.end(), [](World &a, World &b) {
		return a.lastPlayedTimeStamp > b.lastPlayedTimeStamp;
	});

	return worlds;
}

std::string makeWorld(std::string name)
{
	std::string path;
	std::string id;
	while (true) {
		randomID(id);
		path = "worlds/";
		path += id;
		if (!std::filesystem::exists(path)) {
			break;
		}
	}
	std::filesystem::create_directories(path);

	auto now = std::chrono::floor<std::chrono::seconds>(
		std::chrono::system_clock::now());
	auto nowZoned = date::make_zoned(date::current_zone(), now);
	auto nowStr = date::format({}, "%F %T%z", nowZoned);

	auto table = cpptoml::make_table();
	table->insert("name", name);
	table->insert("creation-time", nowStr);
	table->insert("last-played-time", nowStr);
	std::fstream f("worlds/" + id + "/world.toml", std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}

	return id;
}

std::string worldPath(std::string id)
{
	return "worlds/" + id + "/world.swan";
}

std::string thumbnailPath(std::string id)
{
	return "worlds/" + id + "/thumb.png";
}

bool worldExists(std::string id)
{
	return std::filesystem::exists("worlds/" + id);
}

void deleteWorld(std::string id)
{
	std::filesystem::remove_all("worlds/" + id);
}

void renameWorld(std::string id, std::string newName)
{
	auto table = cpptoml::parse_file("worlds/" + id + "/world.toml");
	table->insert("name", newName);
	std::fstream f("worlds/" + id + "/world.toml", std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}
}

void updateWorldLastPlayedTime(std::string id)
{
	auto now = std::chrono::floor<std::chrono::seconds>(
		std::chrono::system_clock::now());
	auto nowZoned = date::make_zoned(date::current_zone(), now);
	auto nowStr = date::format({}, "%F %T%z", nowZoned);

	auto table = cpptoml::parse_file("worlds/" + id + "/world.toml");
	table->insert("last-played-time", nowStr);
	std::fstream f("worlds/" + id + "/world.toml", std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}
}
