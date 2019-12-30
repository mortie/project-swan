#include "EntItemStack.h"

#include <random>

EntItemStack::EntItemStack(const Swan::Context &ctx, const Swan::SRF &params):
		PhysicsEntity(SIZE, MASS) {
	PhysicsEntity::body_.bounciness_ = 0.6;

	readSRF(ctx, params);

	static std::uniform_real_distribution vx(-2.3f, 2.3f);
	static std::uniform_real_distribution vy(-2.3f, -1.2f);

	body_.pos_.y += 0.5 - body_.size_.y / 2;
	body_.vel_ += Swan::Vec2{ vx(ctx.world.random_), vy(ctx.world.random_) };
}

void EntItemStack::draw(const Swan::Context &ctx, Swan::Win &win) {
	SDL_Rect rect = item_->image_.frameRect();
	win.showTexture(body_.pos_, item_->image_.texture_.get(), &rect);
}

void EntItemStack::tick(const Swan::Context &ctx, float dt) {
	despawn_timer_ -= dt;
	if (despawn_timer_ <= 0)
		ctx.plane.despawnEntity(*this);
}

void EntItemStack::readSRF(const Swan::Context &ctx, const Swan::SRF &srf) {
	auto &arr = dynamic_cast<const Swan::SRFArray &>(srf);
	auto *pos = dynamic_cast<Swan::SRFFloatArray *>(arr.val[0].get());
	auto *name = dynamic_cast<Swan::SRFString *>(arr.val[1].get());

	body_.pos_.set(pos->val[0], pos->val[1]);
	item_ = &ctx.world.getItem(name->val);
}

Swan::SRF *EntItemStack::writeSRF(const Swan::Context &ctx) {
	return new Swan::SRFArray{
		new Swan::SRFFloatArray{ body_.pos_.x, body_.pos_.y },
		new Swan::SRFString{ item_->name_ },
	};
}
