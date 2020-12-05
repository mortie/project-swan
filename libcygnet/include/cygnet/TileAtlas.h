#pragma once

#include <memory>

namespace Cygnet {

struct AtlasState;

class TileAtlas {
public:
	TileAtlas();
	~TileAtlas();

	void addTile(size_t tileId, const void *data, size_t len);

private:
	std::unique_ptr<AtlasState> state_;
};

}
