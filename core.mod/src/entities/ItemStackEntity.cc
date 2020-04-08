#include "ItemStackEntity.h"

#include <random>

ItemStackEntity::ItemStackEntity(
		const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item):
			PhysicsEntity(SIZE, MASS) {

	static std::uniform_real_distribution vx(-2.3f, 2.3f);
	static std::uniform_real_distribution vy(-2.3f, -1.2f);

	item_ = &ctx.world.getItem(item);
	body_.pos_ = pos;
	body_.pos_.y += 0.5 - body_.size_.y / 2;
	body_.vel_ += Swan::Vec2{ vx(ctx.world.random_), vy(ctx.world.random_) };
}

ItemStackEntity::ItemStackEntity(const Swan::Context &ctx, const PackObject &obj):
		PhysicsEntity(SIZE, MASS) {
	PhysicsEntity::body_.bounciness_ = 0.6;

	deserialize(ctx, obj);
}

void ItemStackEntity::draw(const Swan::Context &ctx, Swan::Win &win) {
	SDL_Rect rect = item_->image_.frameRect();

	SDL_Texture *tex = item_->image_.texture_.get();
	Swan::TexColorMod darken(tex, 220, 220, 220);

	win.showTexture(body_.pos_, tex, &rect,
		{ .hscale = 0.5, .vscale = 0.5 });
}

void ItemStackEntity::tick(const Swan::Context &ctx, float dt) {
	despawn_timer_ -= dt;
	if (despawn_timer_ <= 0)
		ctx.plane.despawnEntity(*this);
}

void ItemStackEntity::deserialize(const Swan::Context &ctx, const PackObject &obj) {
	body_.pos_ = obj.at("pos").as<Swan::Vec2>();
	item_ = &ctx.world.getItem(obj.at("item").as<std::string>());
}

Swan::Entity::PackObject ItemStackEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone) {
	return {
		{ "pos", msgpack::object(body_.pos_, zone) },
		{ "tile", msgpack::object(item_->name_, zone) },
	};
}
