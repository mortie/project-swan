#include "EntityCollection.h"

#include "WorldPlane.h"

namespace Swan {

void EntityRef::serialize(nbon::Writer w)
{
	w.writeArray([&](nbon::Writer w) {
		w.writeString(coll_->name());
		w.writeUInt(id_);
	});
}

void EntityRef::deserialize(const Context &ctx, nbon::Reader r)
{
	r.getArray([&](nbon::ArrayReader r) {
		coll_ = &ctx.plane.getCollectionOf(r.next().getString());
		id_ = r.next().getUInt();
	});
}

}
