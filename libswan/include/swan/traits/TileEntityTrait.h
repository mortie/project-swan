#pragma once

#include "../common.h"
#include "swan.capnp.h"

namespace Swan {

struct TileEntityTrait {
	struct Tag {};

	struct TileEntity {
		TilePos pos;

		void serialize(proto::TileEntity::Builder w);
		void deserialize(proto::TileEntity::Reader r);
	};

	virtual TileEntity &get(Tag) = 0;

protected:
	~TileEntityTrait() = default;
};

inline void TileEntityTrait::TileEntity::serialize(proto::TileEntity::Builder w)
{
	auto posW = w.initPos();
	posW.setX(pos.x);
	posW.setY(pos.y);
}

inline void TileEntityTrait::TileEntity::deserialize(proto::TileEntity::Reader r)
{
	pos = {r.getPos().getX(), r.getPos().getY()};
}

}
