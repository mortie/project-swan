#include "ItemStackEntity.h"

#include <random>

namespace CoreMod {

ItemStackEntity::ItemStackEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item)
{
	static std::uniform_real_distribution vx(-2.3f, 2.3f);
	static std::uniform_real_distribution vy(-2.3f, -1.2f);

	physicsBody_.body.pos = pos;
	physicsBody_.vel += Swan::Vec2{vx(ctx.world.random_), vy(ctx.world.random_)};
	item_ = &ctx.world.getItem(item);
}

ItemStackEntity::ItemStackEntity(
	const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel, Swan::Item *item)
{
	physicsBody_.body.pos = pos;
	physicsBody_.vel += vel;
	item_ = item;
}

ItemStackEntity::ItemStackEntity(const Swan::Context &ctx, const PackObject &obj)
{
	deserialize(ctx, obj);
}

void ItemStackEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawTile({
		Cygnet::Mat3gf{}.scale({0.5, 0.5}).translate(physicsBody_.body.pos),
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
		ctx.plane.despawnEntity(ctx.plane.currentEntity());
	}
}

void ItemStackEntity::deserialize(const Swan::Context &ctx, const PackObject &obj)
{
	physicsBody_.body.pos = obj.at("pos").as<Swan::Vec2>();
	item_ = &ctx.world.getItem(obj.at("item").as<std::string>());
}

Swan::Entity::PackObject ItemStackEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone)
{
	return {
		{"pos", msgpack::object(physicsBody_.body.pos, zone)},
		{"tile", msgpack::object(item_->name, zone)},
	};
}

}
