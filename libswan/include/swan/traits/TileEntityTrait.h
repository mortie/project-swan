#pragma once

#include "../common.h"

namespace Swan {

struct TileEntityTrait {
	struct Tag {};

	struct TileEntity {
		TilePos pos;
	};

	virtual TileEntity &get(Tag) = 0;

protected:
	~TileEntityTrait() = default;
};

}
