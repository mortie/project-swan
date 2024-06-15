#pragma once

#include <sbon.h>

#include "../common.h"

namespace Swan {

struct TileEntityTrait {
	struct Tag {};

	struct TileEntity {
		TilePos pos;

		void serialize(sbon::Writer w);
		void deserialize(sbon::Reader r);
	};

	virtual TileEntity &get(Tag) = 0;

protected:
	~TileEntityTrait() = default;
};

inline void TileEntityTrait::TileEntity::serialize(sbon::Writer w)
{
	w.writeArray([&](sbon::Writer w) {
		w.writeInt(pos.x);
		w.writeInt(pos.y);
	});
}

inline void TileEntityTrait::TileEntity::deserialize(sbon::Reader r)
{
	r.getArray([&](sbon::ArrayReader r) {
		pos.x = r.next().getInt();
		pos.y = r.next().getInt();
	});
}

}
