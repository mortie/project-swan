#include "EntItemStack.h"

#include <random>

EntItemStack::EntItemStack(const Swan::Context &ctx, const Swan::SRF &params):
		PhysicsEntity(SIZE, MASS) {

	readSRF(ctx, params);

	static std::uniform_real_distribution dis(-0.2f, 0.2f);

	body_.pos_ += Swan::Vec2{ dis(ctx.world.random_), dis(ctx.world.random_) };
}

void EntItemStack::draw(const Swan::Context &ctx, Swan::Win &win) {
	win.setPos(body_.pos_);
	win.draw(sprite_);
}

void EntItemStack::tick(const Swan::Context &ctx, float dt) {
	despawn_timer_ -= dt;
	if (despawn_timer_ <= 0) {
			ctx.plane.despawnEntity(*this);
	}
}

void EntItemStack::readSRF(const Swan::Context &ctx, const Swan::SRF &srf) {
	auto &arr = dynamic_cast<const Swan::SRFArray &>(srf);
	auto *pos = dynamic_cast<Swan::SRFFloatArray *>(arr.val[0].get());
	auto *name = dynamic_cast<Swan::SRFString *>(arr.val[1].get());

	body_.pos_.set(pos->val[0], pos->val[1]);
	item_ = &ctx.world.getItem(name->val);
	tex_.loadFromImage(*item_->image);
	sprite_.setTexture(tex_);
	sprite_.setScale(SIZE);
}

Swan::SRF *EntItemStack::writeSRF(const Swan::Context &ctx) {
	return new Swan::SRFArray{
		new Swan::SRFFloatArray{ body_.pos_.x, body_.pos_.y },
		new Swan::SRFString{ item_->name },
	};
}
