#include "EntItemStack.h"

EntItemStack::EntItemStack(const Swan::Context &ctx, const Swan::SRF &params):
		body_(SIZE, MASS) {

	readSRF(ctx, params);
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
		new Swan::SRFFloatArray{ body_.pos_.x_, body_.pos_.y_ },
		new Swan::SRFString{ item_->name },
	};
}
