#include "EntityCollection.h"

#include "WorldPlane.h"

namespace Swan {

void EntityRef::serialize(sbon::Writer w)
{
	w.writeArray([&](sbon::Writer w) {
		w.writeString(coll_->name());
		w.writeUInt(id_);
	});
}

void EntityRef::deserialize(const Context &ctx, sbon::Reader r)
{
	r.getArray([&](sbon::ArrayReader r) {
		coll_ = &ctx.plane.getCollectionOf(r.next().getString());
		id_ = r.next().getUInt();
	});
}

}
