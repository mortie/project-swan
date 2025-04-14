#include "ItemStackEntity.h"

#include <random>

namespace CoreMod {

ItemStackEntity::ItemStackEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Item *item)
{
	static std::uniform_real_distribution vx(-2.3f, 2.3f);
	static std::uniform_real_distribution vy(-2.3f, -1.2f);

	physicsBody_.body.pos = pos;
	physicsBody_.vel += Swan::Vec2{vx(ctx.world.random_), vy(ctx.world.random_)};
	item_ = item;
}

ItemStackEntity::ItemStackEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel, Swan::Item *item)
{
	physicsBody_.body.pos = pos;
	physicsBody_.vel += vel;
	item_ = item;
}

void ItemStackEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawTile({
		Cygnet::Mat3gf{}
			.translate({0, item_->yOffset})
			.scale({0.5, 0.5})
			.translate(physicsBody_.body.pos),
		item_->id, 0.8,
	});
}

void ItemStackEntity::update(const Swan::Context &ctx, float dt)
{
	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);
}

void ItemStackEntity::tick(const Swan::Context &ctx, float dt)
{
	lifetime_ += dt;
	if (lifetime_ >= DESPAWN_TIME) {
		ctx.plane.entities().despawn(ctx.plane.entities().current());
	}
}

void ItemStackEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	w.setLifetime(lifetime_);
	w.setItem(item_->name);
}

void ItemStackEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	lifetime_ = r.getLifetime();
	item_ = &ctx.world.getItem(r.getItem().cStr());
}

}
