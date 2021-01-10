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

void ItemStackEntity::draw(const Swan::Context &ctx, Swan::Win &win) {
	SDL_Rect rect = item_->image_.frameRect();

	SDL_Texture *tex = item_->image_.texture_.get();
	Swan::TexColorMod darken(tex, 220, 220, 220);

	win.showTexture(body_.pos, tex, &rect,
		{ .hscale = 0.5, .vscale = 0.5 });
}

void ItemStackEntity::update(const Swan::Context &ctx, float dt) {
	physics(ctx, dt, { .mass = MASS, .bounciness = 0.6 });
}

void ItemStackEntity::tick(const Swan::Context &ctx, float dt) {
	despawnTimer_ -= dt;
	if (despawnTimer_ <= 0)
		despawn(ctx);
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
