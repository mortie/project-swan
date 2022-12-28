#include "Entity.h"

#include "WorldPlane.h"

namespace Swan {

void Entity::despawn(const Swan::Context &ctx) {
	onDespawn(ctx);
	ctx.plane.getCollectionOf(typeid(*this)).erase(id_);
}

}
