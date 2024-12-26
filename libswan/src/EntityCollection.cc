#include "EntityCollection.h"

#include "WorldPlane.h"

namespace Swan {

void EntityRef::serialize(proto::EntityRef::Builder w)
{
	w.setCollection(coll_->name());
	w.setId(id_);
}

void EntityRef::deserialize(const Context &ctx, proto::EntityRef::Reader r)
{
	coll_ = ctx.plane.entities().getCollectionOf(r.getCollection().cStr());
	id_ = r.getId();
}

}
