#include "world/seeds.h"
#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "tiles.h"

namespace CoreMod {

template<typename Spec>
class SeedEntity final: public Swan::Entity {
public:
	using Proto = proto::SeedEntity;

	static constexpr float TIMER = 4 * 60;
	static constexpr float TIMER_VARIANCE = 4 * 60;
	static constexpr float DEATH_TIME = 15;

	SeedEntity(Swan::Ctx &ctx) {}
	SeedEntity(Swan::Ctx &ctx, Swan::TilePos pos)
	{
		pos_ = pos;
		timer_ = Swan::randfloat(TIMER, TIMER + TIMER_VARIANCE);
	}

	void tick(Swan::Ctx &ctx, float dt) override
	{
		timer_ -= dt;

		auto &bottom = ctx.plane.tiles().get(pos_.add(0, 1));

		// Spawn grass when the timer runs out
		if (timer_ <= 0) {
			ctx.plane.entities().despawn(ctx.plane.entities().current());

			auto &self = ctx.plane.tiles().get(pos_);
			if (bottom.name == "core::dirt" && !self.isSolid()) {
				ctx.plane.tiles().set(pos_.add(0, 1), "core::grass");
				return;
			}

			bool isAir = self.id == Swan::World::AIR_TILE_ID;
			if (bottom.name == "core::grass" && isAir) {
				ctx.plane.tiles().set(pos_, "core::tall-grass");
				return;
			}

			return;
		}

		// Move the seed out of solid tiles if possible
		if (ctx.plane.tiles().get(pos_).isSolid()) {
			int r = Swan::random() & 3;
			Swan::Vec2i dir;
			switch (r) {
			case 0: dir = {-1, 0}; break;
			case 1: dir = {1, 0}; break;
			default: dir = {0, -1}; break;
			}

			auto pos = pos_ + dir;
			if (!ctx.plane.tiles().get(pos).isSolid()) {
				pos_ = pos;
			}
		}

		// Die after some time on non-grass/dirt
		bool hospitable =
			bottom.name == "core::dirt" ||
			bottom.name == "core::grass";
		if (hospitable && dying_ > 0) {
			dying_ -= dt;
		} else if (!hospitable) {
			dying_ += dt;
			if (dying_ >= DEATH_TIME) {
				ctx.plane.entities().despawn(ctx.plane.entities().current());
				return;
			}
		}
	}

	void serialize(Swan::Ctx &ctx, Proto::Builder w)
	{
		auto pos = w.initPos();
		pos.setX(pos_.x);
		pos.setY(pos_.y);
		w.setPos(pos);
		w.setTimer(timer_);
		w.setDying(dying_);
	}

	void deserialize(Swan::Ctx &ctx, Proto::Reader r)
	{
		pos_.x = r.getPos().getX();
		pos_.y = r.getPos().getY();
		timer_ = r.getTimer();
		dying_ = r.getDying();
	}

private:
	float timer_ = 0;
	float dying_ = 0;
	Swan::TilePos pos_;
};

struct GrassSeedEntitySpec {
	static constexpr Swan::Tile::ID &spawn = tiles::tallGrass;
	static constexpr std::array<std::reference_wrapper<Swan::Tile::ID>, 2> floor = {
		tiles::grass,
		tiles::dirt,
	};
};
using GrassSeedEntity = SeedEntity<GrassSeedEntitySpec>;

struct ScorchbloomSeedEntitySpec {
	static constexpr Swan::Tile::ID &spawn = tiles::scorchbloom;
	static constexpr std::array<std::reference_wrapper<Swan::Tile::ID>, 1> floor = {
		tiles::snow,
	};
};
using scorchbloomSeedEntity = SeedEntity<ScorchbloomSeedEntitySpec>;

void registerSeedEntities(Swan::Mod &mod)
{
	mod.registerEntity<GrassSeedEntity>("grass-seed");
	mod.registerEntity<scorchbloomSeedEntity>("scorchbloom-seed");
}

void spawnGrassSeed(Swan::Ctx &ctx, Swan::TilePos pos)
{
	ctx.plane.entities().spawn<GrassSeedEntity>(pos);
}

void spawnScorchbloomSeed(Swan::Ctx &ctx, Swan::TilePos pos)
{
	ctx.plane.entities().spawn<scorchbloomSeedEntity>(pos);
}

}
