#include "platform.h"

#include <memory>

#include "util.h"

namespace CoreMod {

struct PlatformTraits: Swan::Tile::Traits {};
struct SupportPlatformTraits: PlatformTraits {};

static bool spawnPlatform(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &left = ctx.plane.tiles().get(pos.add(-1, 0));
	auto &right = ctx.plane.tiles().get(pos.add(1, 0));
	if (left.isSupportH() || right.isSupportH()) {
		return true;
	}

	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	return below.isSolid();
}

static void updatePlatform(Swan::Ctx &ctx, Swan::TilePos pos)
{
	auto &below = ctx.plane.tiles().get(pos.add(0, 1));
	auto &left = ctx.plane.tiles().get(pos.add(-1, 0));
	auto &right = ctx.plane.tiles().get(pos.add(1, 0));

	if (!below.isSolid() && !left.isSupportH() && !right.isSupportH()) {
		breakTileAndDropItem(ctx, pos);
		return;
	}

	auto &self = ctx.plane.tiles().get(pos);
	bool hasSolidBelow = below.isSolid();
	bool hasPlatformLeft = dynamic_cast<PlatformTraits *>(left.more->traits.get());
	bool hasSolidLeft = left.isSupportH() && !hasPlatformLeft;
	bool hasPlatformRight = dynamic_cast<PlatformTraits *>(right.more->traits.get());
	bool hasSolidRight = right.isSupportH() && !hasPlatformRight;

	std::string_view desired = "";
	if (hasSolidBelow) {
		if (hasPlatformLeft && hasPlatformRight) {
			desired = "core::platform::supported::center";
		} else if (hasPlatformLeft) {
			desired = "core::platform::supported::right";
		} else if (hasPlatformRight) {
			desired = "core::platform::supported::left";
		} else {
			desired = "core::platform::supported::lone";
		}
	} else {
		if (hasPlatformLeft && hasPlatformRight) {
			desired = "core::platform::center";
		} else if (hasPlatformLeft && hasSolidRight) {
			desired = "core::platform::anchored::right";
		} else if (hasSolidLeft && hasPlatformRight) {
			desired = "core::platform::anchored::left";
		} else if (hasPlatformLeft) {
			desired = "core::platform::right";
		} else if (hasPlatformRight) {
			desired = "core::platform::left";
		} else if (hasSolidLeft) {
			desired = "core::platform::anchored::stub::right";
		} else if (hasSolidRight) {
			desired = "core::platform::anchored::stub::left";
		}
	}

	if (desired == "") {
		breakTileAndDropItem(ctx, pos);
		return;
	}

	if (self.name == desired) {
		return;
	}

	auto &desiredSelf = ctx.world.getTile(desired);

	auto tileIsPlatform = [](Swan::Tile &t) {
		auto ptr = dynamic_cast<PlatformTraits *>(t.more->traits.get());
		return ptr != nullptr;
	};

	auto tileIsSupport = [](Swan::Tile &t) {
		auto ptr = dynamic_cast<SupportPlatformTraits *>(t.more->traits.get());
		return ptr != nullptr;
	};

	// Search for supporting tiles nearby
	bool isSupported = tileIsSupport(desiredSelf);
	if (!isSupported) {
		bool continueLeft = true;
		bool continueRight = true;
		for (int x = 1; x < 20 && (continueLeft || continueRight); ++x) {
			if (continueLeft) {
				auto &left = ctx.plane.tiles().get(pos.add(-x, 0));
				if (tileIsSupport(left)) {
					isSupported = true;
					break;
				} else if (!tileIsPlatform(left)) {
					continueLeft = false;
				}
			}

			if (continueRight) {
				auto &right = ctx.plane.tiles().get(pos.add(x, 0));
				if (tileIsSupport(right)) {
					isSupported = true;
					break;
				} else if (!tileIsPlatform(right)) {
					continueRight = false;
				}
			}
		}
	}

	if (isSupported) {
		ctx.plane.tiles().setID(pos, desiredSelf.id);
	} else {
		breakTileAndDropItem(ctx, pos);
	}
}

void registerPlatform(Swan::Mod &mod)
{
	auto traits = std::make_shared<PlatformTraits>();
	auto supportTraits = std::make_shared<SupportPlatformTraits>();

	Swan::Tile::Builder tile = {
		.isSolid = false,
		.isSupportV = true,
		.isSupportH = true,
		.isPlatform = true,
		.isFullSupportH = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::platform",
		.onSpawn = spawnPlatform,
		.onTileUpdate = updatePlatform,
	};

	mod.registerTile(tile
		.withName("platform::left")
		.withImage("core::tiles/platform@0")
		.withTraits(traits));
	mod.registerTile(tile
		.withName("platform::right")
		.withImage("core::tiles/platform@2")
		.withTraits(traits));
	mod.registerTile(tile
		.withName("platform::center")
		.withImage("core::tiles/platform@1")
		.withTraits(traits));

	mod.registerTile(tile
		.withName("platform::anchored::stub::left")
		.withImage("core::tiles/platform@3")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform::anchored::stub::right")
		.withImage("core::tiles/platform@5")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform")
		.withImage("core::tiles/platform@4")
		.withTraits(traits));

	mod.registerTile(tile
		.withName("platform::anchored::left")
		.withImage("core::tiles/platform@6")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform::anchored::right")
		.withImage("core::tiles/platform@8")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform::supported::lone")
		.withImage("core::tiles/platform@7")
		.withTraits(supportTraits));

	mod.registerTile(tile
		.withName("platform::supported::left")
		.withImage("core::tiles/platform@9")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform::supported::right")
		.withImage("core::tiles/platform@11")
		.withTraits(supportTraits));
	mod.registerTile(tile
		.withName("platform::supported::center")
		.withImage("core::tiles/platform@10")
		.withTraits(supportTraits));
}

}
