#pragma once

#include "../common.h"
#include "swan.capnp.h"

namespace Swan {

struct TileEntity {
	TilePos pos{};
	bool keep = false;

	void serialize(proto::TileEntity::Builder w);
	void deserialize(proto::TileEntity::Reader r);
};

struct TileEntityTrait {
	struct Tag {};

	virtual TileEntity &get(Tag) = 0;

protected:
	~TileEntityTrait() = default;
};

inline void TileEntity::serialize(proto::TileEntity::Builder w)
{
	auto posW = w.initPos();
	posW.setX(pos.x);
	posW.setY(pos.y);
}

inline void TileEntity::deserialize(proto::TileEntity::Reader r)
{
	pos = {r.getPos().getX(), r.getPos().getY()};
}

}
