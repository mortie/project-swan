#include "EntityCollection.h"

#include "WorldPlane.h"

namespace Swan {

void EntityRef::serialize(proto::EntityRef::Builder w)
{
	if (coll_) {
		w.setCollection(coll_->name());
		w.setId(id_);
	} else {
		w.setCollection(nullptr);
	}
}

void EntityRef::deserialize(Ctx &ctx, proto::EntityRef::Reader r)
{
	if (r.hasCollection()) {
		coll_ = ctx.plane.entities().getCollectionOf(r.getCollection().cStr());
		id_ = r.getId();
	} else {
		coll_ = nullptr;
		id_ = 0;
	}
}

}
