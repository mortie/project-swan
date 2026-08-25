#include <algorithm>
#include <string_view>
#include <swan/swan.h>
#include <iostream>
#include <utility>

std::string_view varify(std::string_view name)
{
	static thread_local std::string varName;
	varName.clear();
	for (size_t i = 0; i < name.size(); ++i) {
		char ch = name[i];
		if (ch == '-' && i < name.size() - 1) {
			varName += char(toupper(name[i + 1]));
			i += 1;
		} else if (ch == '/') {
			varName += "__";
		} else if (ch == ':' || ch == '-') {
			varName += '_';
		} else {
			varName += ch;
		}
	}

	return std::string_view(varName);
};

void enumerateSprites(std::vector<std::string> &out, std::string base, std::string path)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newPath = Swan::cat(path, "/", it.path().filename());
			std::string newBase = Swan::cat(base, it.path().filename(), "/");
			enumerateSprites(out, std::move(newBase), std::move(newPath));
			continue;
		}

		auto ext = it.path().filename().extension();
		if (ext == ".txt" || ext == ".toml") {
			continue;
		} else if (ext != ".png") {
			Swan::warn << it.path() << ": Unknown extension " << ext;
			continue;
		}

		std::string name = Swan::cat(base, it.path().filename().stem());
		out.push_back(std::move(name));
	}
}

void enumerateSounds(std::vector<std::string> &out, std::string base, std::string path)
{
	if (!std::filesystem::exists(path)) {
		return;
	}

	std::vector<std::string> sounds;
	for (auto &it: std::filesystem::directory_iterator(path)) {
		if (it.is_directory()) {
			std::string newPath = Swan::cat(path, "/", it.path().filename());
			std::string newBase = Swan::cat(base, it.path().filename(), "/");
			enumerateSounds(out, std::move(newBase), std::move(newPath));
			continue;
		}

		auto ext = it.path().filename().extension();
		if (ext == ".txt") {
			continue;
		} else if (ext != ".ogg") {
			Swan::warn << it.path() << ": Unknown extension " << ext;
			continue;
		}

		std::string name = Swan::cat(base, it.path().filename().stem());
		out.push_back(std::move(name));
	}
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cout << "Usage: " << argv[0] << " <category> <mod>\n";
		return 1;
	}

	std::string category = argv[1];
	std::string_view modPath = argv[2];

	std::string dlpath = Swan::cat(modPath, "/.swanbuild/mod");

	Swan::OS::Dynlib dl(dlpath);
	auto create = dl.get<Swan::ModCreateFn>("mod_create");
	if (!create) {
		std::cerr << modPath << ": No 'mod_create' function!";
		return 1;
	}

	Swan::ModWrapper wrapper;
	wrapper.path_ = modPath;
	std::unique_ptr<Swan::Mod> mod(create(wrapper));

	std::vector<std::string> names;

	if (category == "tiles") {
		for (auto &tile: mod->tiles_) {
			if (tile.name.find('@') != std::string::npos) {
				continue;
			}
			names.push_back(tile.name);
		}
	} else if (category == "actions") {
		for (auto &spec: mod->actions_) {
			names.push_back(spec.name);
		}
	} else if (category == "sprites") {
		enumerateSprites(names, "", Swan::cat(modPath, "/assets/sprites"));
	} else if (category == "sounds") {
		enumerateSounds(names, "", Swan::cat(modPath, "/assets/sounds"));
	} else {
		std::cerr << "Unknown category: " << category << '\n';
		return 1;
	}

	std::sort(names.begin(), names.end());
	for (auto &name: names) {
		std::cout << "X(" << varify(name) << ", \"" << mod->name_ << "::" << name << "\");\n";
	}

	return 0;
}
