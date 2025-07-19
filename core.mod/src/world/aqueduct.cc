#include "aqueduct.h"
#include "swan/util.h"
#include "tileentities/AqueductTileEntity.h"

namespace CoreMod {

static void updateAqueductTileEntity(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto ent = ctx.plane.entities().getTileEntity(pos).as<AqueductTileEntity>();
	if (!ent) {
		Swan::warn << "Missing aqueduct tile entity for aqueduct tile!";
		return;
	}

	ent->onTileUpdate(ctx);
}

static void updateAqueduct(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &leftTile = ctx.plane.tiles().get(pos.add(-1, 0));
	auto left = dynamic_cast<AqueductTrait *>(leftTile.traits.get());
	bool hasLeft = left && left->connectable.has(Swan::Direction::RIGHT);

	auto &rightTile = ctx.plane.tiles().get(pos.add(1, 0));
	auto right = dynamic_cast<AqueductTrait *>(rightTile.traits.get());
	bool hasRight = right && right->connectable.has(Swan::Direction::LEFT);

	if (hasLeft && hasRight) {
		ctx.plane.tiles().set(pos, "core::aqueduct::center");
	} else if (hasLeft) {
		ctx.plane.tiles().set(pos, "core::aqueduct::right");
	} else if (hasRight) {
		ctx.plane.tiles().set(pos, "core::aqueduct::left");
	} else {
		ctx.plane.tiles().set(pos, "core::aqueduct");
	}

	updateAqueductTileEntity(ctx, pos);
}

template<Swan::FixedString newName>
void activateAqueduct(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta)
{
	ctx.plane.tiles().set(pos, newName);
}

void registerAqueduct(Swan::Mod &mod)
{
	constexpr auto NONE = Swan::DirectionSet{};
	constexpr auto LEFT = Swan::Direction::LEFT;
	constexpr auto RIGHT = Swan::Direction::RIGHT;

	auto fc = [](unsigned long long pattern) {
		return std::make_shared<Swan::FluidCollision>(pattern);
	};

	mod.registerEntity<AqueductTileEntity>("tile::aqueduct");

	mod.registerTile({
		.name = "aqueduct",
		.image = "core::tiles/aqueduct@4",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueduct,
		.fluidCollision = fc(0b0110'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(LEFT | RIGHT, NONE),
	});

	mod.registerTile({
		.name = "aqueduct::center",
		.image = "core::tiles/aqueduct@1",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueduct,
		.fluidCollision = fc(0b1111'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(LEFT | RIGHT, LEFT | RIGHT),
	});

	mod.registerTile({
		.name = "aqueduct::left",
		.image = "core::tiles/aqueduct@0",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueduct,
		.onActivate = activateAqueduct<"core::aqueduct::mouth::left">,
		.fluidCollision = fc(0b1110'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(LEFT | RIGHT, RIGHT),
	});

	mod.registerTile({
		.name = "aqueduct::right",
		.image = "core::tiles/aqueduct@2",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueduct,
		.onActivate = activateAqueduct<"core::aqueduct::mouth::right">,
		.fluidCollision = fc(0b0111'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(LEFT | RIGHT, LEFT),
	});

	mod.registerTile({
		.name = "aqueduct::mouth::left",
		.image = "core::tiles/aqueduct@3",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueductTileEntity,
		.onActivate = activateAqueduct<"core::aqueduct::left">,
		.fluidCollision = fc(0b1110'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(RIGHT, LEFT | RIGHT),
	});

	mod.registerTile({
		.name = "aqueduct::mouth::right",
		.image = "core::tiles/aqueduct@5",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::aqueduct",
		.tileEntity = "core::tile::aqueduct",
		.onTileUpdate = updateAqueductTileEntity,
		.onActivate = activateAqueduct<"core::aqueduct::right">,
		.fluidCollision = fc(0b0111'0000'0000'0000),
		.traits = std::make_shared<AqueductTrait>(LEFT, LEFT | RIGHT),
	});
}

}
