#include "EntityCollection.h"

#include "WorldPlane.h"

namespace Swan {

void EntityRef::serialize(MsgStream::Serializer &w)
{
	auto arr = w.beginArray(2);

	arr.writeString(coll_->name());
	arr.writeUInt(id_);
	w.endArray(arr);
}

void EntityRef::deserialize(const Context &ctx, MsgStream::Parser &r)
{
	auto arr = r.nextArray();

	coll_ = &ctx.plane.getCollectionOf(arr.nextString());
	id_ = arr.nextUInt();
	arr.skipAll();
}

}
