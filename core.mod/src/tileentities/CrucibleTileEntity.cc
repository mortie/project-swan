#include "CrucibleTileEntity.h"
#include "entities/ItemStackEntity.h"

namespace CoreMod {

void CrucibleTileEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	if (!drawSupports_) {
		return;
	}

	rnd.drawSprite(Cygnet::RenderLayer::BEHIND, {
		.transform = Cygnet::Mat3gf{}
			.translate(tileEntity_.pos.as<float>().add(0, 1)),
		.sprite = sprite_,
	});
}

void CrucibleTileEntity::tick(const Swan::Context &ctx, float dt)
{
}

void CrucibleTileEntity::serialize(const Swan::Context &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	w.setDrawSupports(drawSupports_);
	w.setTepmerature(temperature_);
}

void CrucibleTileEntity::deserialize(const Swan::Context &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	drawSupports_ = r.getDrawSupports();
	temperature_ = r.getTepmerature();
}

}
