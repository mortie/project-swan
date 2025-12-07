#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class GrassSeedEntity final: public Swan::Entity {
public:
	using Proto = proto::GrassSeedEntity;

	GrassSeedEntity(Swan::Ctx &ctx) {}
	GrassSeedEntity(
		Swan::Ctx &ctx, Swan::TilePos pos);

	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	float timer_ = 0;
	float dying_ = 0;
	Swan::TilePos pos_;
};

}
