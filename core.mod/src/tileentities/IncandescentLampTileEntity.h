#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "traits/PowerNodeTrait.h"

namespace CoreMod {

class IncandescentLampTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public PowerNodeTrait {
public:
	IncandescentLampTileEntity(Swan::Ctx &ctx)
	{}

	Swan::TileEntity &get(Swan::TileEntityTrait::Tag) override
	{ return tileEntity_; }

	PowerNode &get(PowerNodeTrait::Tag) override
	{ return powerNode_; }

	void tick(Swan::Ctx &ctx, float dt) override;
	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void onDespawn(Swan::Ctx &ctx) override;
	void drawDebug(Swan::Ctx &ctx) override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

private:
	float kelvin() { return temperature_ + 300; }

	Swan::TileEntity tileEntity_;
	PowerNode powerNode_{{24.5/32, 29.5/32}};

	// Temperature, represented as "degrees kelvin over ambient".
	// "Ambient" is assumed to be 300k (~27C).
	// The numbers will be in the thousands before any light appears,
	// so the exact ambient temperature doesn't matter and 300k
	// is a nice round number.
	float temperature_ = 0;

	float light_ = 0;
};

}
