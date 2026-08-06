#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "swan/EntityCollection.h"

namespace CoreMod {

class CopperWireEntity: public Swan::Entity {
public:
	CopperWireEntity(Swan::Ctx &)
	{}

	CopperWireEntity(Swan::Ctx &, Swan::Vec2 startPoint):
		points_{startPoint}
	{}

	using Proto = proto::CopperWireEntity;

	bool setEndPoint(Swan::Vec2 endPoint);

	void update(Swan::Ctx &ctx, float dt) override;
	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void onDespawn(Swan::Ctx &ctx) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	Swan::EntityRef begin_;
	Swan::EntityRef end_;

private:
	void simulatePhysicsStep(Swan::Ctx &ctx, float dt);
	void performCollisions(Swan::Ctx &ctx);

	// Store all points, including the fixed (start and end) points
	std::vector<Swan::Vec2> points_;
	float timer_ = 0;
};

}
