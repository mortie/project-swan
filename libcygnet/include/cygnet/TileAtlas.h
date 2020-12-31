#pragma once

#include <memory>

namespace Cygnet {

struct AtlasState;

class TileAtlas {
public:
	TileAtlas();
	~TileAtlas();

	void addTile(size_t tileId, const void *data);
	const unsigned char *getImage(size_t *w, size_t *h);

private:
	std::unique_ptr<AtlasState> state_;
};

}
