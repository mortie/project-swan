#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "Tile.h"

namespace Swan {

class Mod {
public:
	using ModID = uint32_t;

	std::string name_;
	std::string path_;
	std::vector<Tile> tiles_;
	bool inited_ = false;

	void init(const std::string &name);
	void registerTile(const std::string &name, const std::string &asset);
};

}
