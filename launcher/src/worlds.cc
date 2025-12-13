#include "worlds.h"
#include "swan/log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cpptoml.h>

#include <stdexcept>

static constexpr const char *identifier = "coffee.mort.Swan";

static std::filesystem::path dataDir()
{
	namespace fs = std::filesystem;

	fs::path path;
#if defined(__APPLE__)
	const char *home = getenv("HOME");
	if (!home || home[0] == '\0') {
		throw std::runtime_error("Home dir not set");
	}

	path = fs::path(home) / "Library" / "Application Support" / identifier;
#elif defined(__linux__)
	const char *data = getenv("XDG_DATA_DIR");
	if (data && data[0]) {
		path = fs::path(data) / identifier;
	} else {
		const char *home = getenv("HOME");
		if (!home) {
			throw std::runtime_error("Home dir not set");
		}

		path = fs::path(home) / ".local" / "share" / identifier;
	}
#elif defined(__MINGW32__)
	// TODO
	path = ".";
#else
#error "Unsupported platform"
#endif

	Swan::info << "Using data directory: " << path;
	return path;
}

static std::filesystem::path worldsDir = dataDir() / "worlds";

static std::string formatNow()
{
	time_t now = std::time(nullptr);
	std::tm tm = *localtime(&now);

	std::string s;
	s.resize(256);
	size_t n = std::strftime(s.data(), s.size(), "%Y:%m:%d %H:%M:%S", &tm);
	s.resize(n);
	return s;
}

static void randomID(std::string &str)
{
	constexpr const char *alphabet = "0123456789abcdef";
	str.resize(16);
	std::generate_n(str.begin(), str.size(), [] { return alphabet[rand() % 16]; });
}

std::vector<World> listWorlds()
{
	if (!std::filesystem::exists(worldsDir)) {
		return {};
	}

	std::vector<World> worlds;

	for (auto &ent: std::filesystem::directory_iterator(worldsDir)) {
		std::string id = ent.path().filename().string();
		auto tomlPath = worldsDir / id / "world.toml";

		auto table = cpptoml::parse_file(tomlPath.string());
		auto name = table->get_as<std::string>("name");
		auto creationTime = table->get_as<std::string>("creation-time");
		auto lastPlayedTime = table->get_as<std::string>("last-played-time");

		worlds.push_back({
			.id = id,
			.name = name.value_or("Untitled"),
			.creationTime = creationTime.value_or("Unknown"),
			.lastPlayedTime = lastPlayedTime.value_or("Unknown"),
		});
	}

	std::sort(worlds.begin(), worlds.end(), [](World &a, World &b) {
		return strcmp(
			a.lastPlayedTime.c_str(),
			b.lastPlayedTime.c_str()) > 0;
	});

	return worlds;
}

std::string makeWorld(std::string name)
{
	std::filesystem::path path;
	std::string id;
	while (true) {
		randomID(id);
		path = worldsDir / id;
		if (!std::filesystem::exists(path)) {
			break;
		}
	}
	std::filesystem::create_directories(path);
	auto tomlPath = path / "world.toml";

	auto nowStr = formatNow();
	auto table = cpptoml::make_table();
	table->insert("name", name);
	table->insert("creation-time", nowStr);
	table->insert("last-played-time", nowStr);
	std::fstream f(tomlPath, std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}

	return id;
}

std::string worldPath(std::string_view id)
{
	return (worldsDir / id / "world.swan").string();
}

std::string thumbnailPath(std::string_view id)
{
	return (worldsDir / id / "thumb.png").string();
}

bool worldExists(std::string_view id)
{
	return std::filesystem::exists(worldsDir / id);
}

void deleteWorld(std::string_view id)
{
	std::filesystem::remove_all(worldsDir / id);
}

void renameWorld(std::string_view id, std::string newName)
{
	auto tomlPath = worldsDir / id / "world.toml";
	auto table = cpptoml::parse_file(tomlPath.string());
	table->insert("name", std::move(newName));
	std::fstream f(tomlPath, std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}
}

void updateWorldLastPlayedTime(std::string_view id)
{
	auto nowStr = formatNow();
	auto tomlPath = worldsDir / id / "world.toml";
	auto table = cpptoml::parse_file(tomlPath.string());
	table->insert("last-played-time", nowStr);
	std::fstream f(tomlPath, std::ios_base::out);
	f << *table;
	f.close();
	if (f.fail()) {
		throw std::runtime_error("Failed to write world toml");
	}
}
