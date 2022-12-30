#include "ItemStackEntity.h"

#include <random>

ItemStackEntity::ItemStackEntity(
		const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item):
			ItemStackEntity() {

	static std::uniform_real_distribution vx(-2.3f, 2.3f);
	static std::uniform_real_distribution vy(-2.3f, -1.2f);

	body_.pos = pos;
	item_ = &ctx.world.getItem(item);
	physics_.vel += Swan::Vec2{ vx(ctx.world.random_), vy(ctx.world.random_) };
}

ItemStackEntity::ItemStackEntity(const Swan::Context &ctx, const PackObject &obj):
		ItemStackEntity() {
	deserialize(ctx, obj);
}

void ItemStackEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) {
	rnd.drawTile(item_->id, Cygnet::Mat3gf{}.scale({0.5, 0.5}).translate(body_.pos), 0.8);
}

void ItemStackEntity::update(const Swan::Context &ctx, float dt) {
	physics(ctx, dt, { .mass = MASS, .bounciness = 0.6 });
}

void ItemStackEntity::tick(const Swan::Context &ctx, float dt) {
	despawnTimer_ -= dt;
	if (despawnTimer_ <= 0) {
		ctx.plane.despawnEntity(ctx.plane.currentEntity());
	}
}

void ItemStackEntity::deserialize(const Swan::Context &ctx, const PackObject &obj) {
	body_.pos = obj.at("pos").as<Swan::Vec2>();
	item_ = &ctx.world.getItem(obj.at("item").as<std::string>());
}

Swan::Entity::PackObject ItemStackEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone) {
	return {
		{ "pos", msgpack::object(body_.pos, zone) },
		{ "tile", msgpack::object(item_->name, zone) },
	};
}
