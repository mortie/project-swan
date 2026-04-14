#include <swan/swan.h>
#include <iostream>

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <mod>\n";
		return 1;
	}

	std::string dlpath = Swan::cat(argv[1], "/.swanbuild/mod");

	Swan::OS::Dynlib dl(dlpath);
	auto create = dl.get<Swan::Mod *(*)()>("mod_create");
	if (!create) {
		std::cerr << argv[1] << ": No 'mod_create' function!";
		return 1;
	}

	std::unique_ptr<Swan::Mod> mod(create());

	std::string varName;
	for (auto &tile: mod->tiles_) {
		if (tile.name.find('@') != std::string::npos) {
			continue;
		}

		varName.clear();
		for (size_t i = 0; i < tile.name.size(); ++i) {
			char ch = tile.name[i];
			if (ch == '-' && i < tile.name.size() - 1) {
				varName += char(toupper(tile.name[i + 1]));
				i += 1;
			} else if (ch == ':' || ch == '-') {
				varName += '_';
			} else {
				varName += ch;
			}
		}

		std::cout << "X(" << varName << ", \"" << mod->name_ << "::" << tile.name << "\");\n";
	}

	return 0;
}
