#include "World.h"

#include <limits>
#include <string_view>

#include "log.h"
#include "Game.h"
#include "Clock.h"
#include "assets.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep
#include "swan/constants.h"

namespace Swan {

static void chunkLine(int l, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir)
{
	for (int i = 0; i < l; ++i) {
		plane.slowGetChunk(abspos).keepActive();
		abspos += dir;
	}
}

std::vector<ModWrapper> World::loadMods(std::span<const std::string> paths)
{
	std::vector<ModWrapper> mods;

	mods.reserve(paths.size());

	for (auto &path: paths) {
		OS::Dynlib dl(path + "/.swanbuild/mod");
		auto create = dl.get<Mod *(*)()>("mod_create");
		if (create == NULL) {
			warn << path << ": No 'mod_create' function!";
			continue;
		}

		std::unique_ptr<Mod> mod(create());
		mods.push_back(ModWrapper(std::move(mod), path, std::move(dl)));
	}

	return mods;
}

void World::buildResources()
{
	Cygnet::ResourceBuilder builder(&game_->renderer_);

	// We need default all-filled fluid collision
	auto solidFluidCollision = std::make_shared<FluidCollision>();
	solidFluidCollision->set();

	// We need a fallback image for when an image is missing
	unsigned char tileImageBuffer[TILE_SIZE * TILE_SIZE * 4];
	for (size_t i = 0; i < TILE_SIZE * TILE_SIZE; ++i) {
		tileImageBuffer[i * 4 + 0] = PLACEHOLDER_RED;
		tileImageBuffer[i * 4 + 1] = PLACEHOLDER_GREEN;
		tileImageBuffer[i * 4 + 2] = PLACEHOLDER_BLUE;
		tileImageBuffer[i * 4 + 3] = 255;
	}

	// Built-in tile asset
	builder.addTileAsset(INVALID_TILE_NAME, tileImageBuffer, 1);

	// Built-in sprite
	sprites_[INVALID_SPRITE_NAME] = builder.addSprite(tileImageBuffer, {
		.width = TILE_SIZE,
		.height = TILE_SIZE,
		.frameHeight = TILE_SIZE,
		.repeatFrom = 0,
	});

	// Re-use tile image buffer to create an air image asset
	memset(tileImageBuffer, 0, TILE_SIZE * TILE_SIZE * 4);
	builder.addTileAsset(AIR_TILE_NAME, tileImageBuffer, 1);

	// Built-in sounds
	sounds_[INVALID_SOUND_NAME] = {};
	loadSoundAssets("@::", "./assets/sounds", sounds_);

	// Fallback particles for tiles which don't have an image
	auto fallbackTileParticles = std::make_shared<TileParticles>();

	HashMap<TileAssetMeta> tileMeta;
	HashMap<Cygnet::RenderMask> masks;

	// Load assets from mods
	for (auto &mod: mods_) {
		loadSoundAssets(
			cat(mod.name(), "::"),
			cat(mod.path_, "/assets/sounds"),
			sounds_);
		loadTileAssets(
			cat(mod.name(), "::tiles/"),
			cat(mod.path_, "/assets/tiles"),
			builder, tileMeta);
		loadTileAssets(
			cat(mod.name(), "::items/"),
			cat(mod.path_, "/assets/items"),
			builder, tileMeta);
		loadSpriteAssets(
			cat(mod.name(), "::"),
			cat(mod.path_, "/assets/sprites"),
			builder, sprites_);
		loadMaskAssets(
			cat(mod.name(), "::masks/"),
			cat(mod.path_, "/assets/masks"),
			builder, masks);
	}

	// After this point, 'sounds_' *must* be unchanged.
	// We rely on pointers to be stable from now on.
	SoundAsset *fallbackSound = &sounds_[INVALID_SOUND_NAME];
	SoundAsset *thudSound = getSound(THUD_SOUND_NAME);

	// Let tile ID 0 be the invalid tile
	builder.addTile(INVALID_TILE_ID, INVALID_TILE_NAME);
	tilesMap_[INVALID_TILE_NAME] = INVALID_TILE_ID;
	tiles_.push_back(Tile(INVALID_TILE_ID, INVALID_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
		.isSolid = false,
		.isReplacable = true,
	}));
	items_.push_back(Item(INVALID_TILE_ID, INVALID_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
	}));
	itemsMap_[INVALID_TILE_NAME] = INVALID_TILE_ID;

	// ...And tile ID 1 be the air tile
	builder.addTile(AIR_TILE_ID, AIR_TILE_NAME);
	tilesMap_[AIR_TILE_NAME] = AIR_TILE_ID;
	tiles_.push_back(Tile(AIR_TILE_ID, AIR_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
		.isSolid = false,
		.isReplacable = true,
	}));
	items_.push_back(Item(AIR_TILE_ID, AIR_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
	}));
	itemsMap_[AIR_TILE_NAME] = AIR_TILE_ID;

	// Set attributes for all built-in tiles
	for (auto &tile: tiles_) {
		tile.more->stepSounds[0] = fallbackSound;
		tile.more->stepSounds[1] = fallbackSound;
		tile.more->placeSound = fallbackSound;
		tile.more->breakSound = fallbackSound;
		tile.more->particles = fallbackTileParticles;
	}

	// Count number of tiles.
	// This lets us avoid re-allocating the tiles vector
	// while building tiles.
	size_t tileCount = tiles_.size();
	for (auto &mod: mods_) {
		tileCount += mod.mod_->tiles_.size();
	}

	// Need to fill in every tile before we do items,
	// because all items will end up after all tiles in the tile atlas.
	// In the rendering system, there's no real difference between a tile
	// and an item.
	tiles_.reserve(tileCount);
	items_.reserve(tileCount);
	for (auto &mod: mods_) {
		for (auto &tileBuilder: mod.mod_->tiles_) {
			std::string tileName = cat(mod.name(), "::", tileBuilder.name);
			Tile::ID tileId = tiles_.size();

			auto metaIt = tileMeta.find(tileBuilder.image);

			tilesMap_[tileName] = tileId;
			tiles_.push_back(Tile(tileId, tileName, std::move(tileBuilder)));
			auto &tile = tiles_.back();

			if (tileBuilder.fluidMask) {
				auto maskIt = masks.find(*tileBuilder.fluidMask);
				if (maskIt == masks.end()) {
					warn
						<< "Tile " << tileName << " referenced unknown mask "
						<< *tileBuilder.fluidMask;
				} else {
					tile.more->fluidMask = maskIt->second;
				}
			}

			if (!tile.more->fluidCollision && (tile.isSolid())) {
				tile.more->fluidCollision = solidFluidCollision;
			}

			float yOffset = 0;
			if (metaIt != tileMeta.end()) {
				tile.more->particles = metaIt->second.particles;
				yOffset = metaIt->second.yOffset;
			} else {
				tile.more->particles = fallbackTileParticles;
			}
			builder.addTile(tileId, tileBuilder.image);

			if (tileBuilder.placeSound) {
				tile.more->placeSound = getSound(tileBuilder.placeSound.value());
			}
			else {
				tile.more->placeSound = thudSound;
			}

			if (tileBuilder.breakSound) {
				tile.more->breakSound = getSound(tileBuilder.breakSound.value());
			}
			else {
				tile.more->breakSound = tile.more->placeSound;
			}

			if (tileBuilder.stepSound) {
				auto &s = tileBuilder.stepSound.value();
				tile.more->stepSounds[0] = getSound(cat(s, "1"));
				tile.more->stepSounds[1] = getSound(cat(s, "2"));
			}
			else if (tile.isSolid()) {
				tile.more->stepSounds[0] = thudSound;
				tile.more->stepSounds[1] = thudSound;
			}
			else {
				tile.more->stepSounds[0] = fallbackSound;
				tile.more->stepSounds[1] = fallbackSound;
			}

			/*
			 * Create item representing the tile
			 */

			auto name = tile.name.string();
			items_.push_back(Item(tile.id, name, {
				.name = "",
				.lightLevel = tile.more->lightLevel,
			}));
			auto &item = items_.back();
			itemsMap_[name] = tile.id;
			item.displayName = mod.lang("items", tileBuilder.name);
			item.tile = &tile;
			item.yOffset = yOffset;

			// Tiles whose names contain '::' are "variants".
			// Convention is to have one tile without a '::' which represents
			// the "logical" tile, and then make variants with '::' in the name.
			item.hidden = tileBuilder.name.find("::") != std::string::npos;
		}
	}

	// Put all items after all the tiles
	Tile::ID nextItemId = tileCount;
	assert(nextItemId == tiles_.size());

	// Load all items which aren't just tiles in disguise.
	for (auto &mod: mods_) {
		for (auto &itemBuilder: mod.mod_->items_) {
			//auto image = loadTileImage(itemBuilder.image);

			std::string itemName = cat(mod.name(), "::", itemBuilder.name);
			Tile::ID itemId = nextItemId++;

			auto metaIt = tileMeta.find(itemBuilder.image);

			float yOffset = 0;
			if (metaIt != tileMeta.end()) {
				yOffset = metaIt->second.yOffset;
			}
			builder.addTile(itemId, itemBuilder.image);

			items_.push_back(Item(itemId, itemName, itemBuilder));
			itemsMap_[itemName] = itemId;
			auto &item = items_.back();
			item.displayName = mod.lang("items", itemBuilder.name);
			item.yOffset = yOffset;
			item.hidden = false;
			if (itemBuilder.tile) {
				item.tile = &getTile(itemBuilder.tile.value());
			}
		}
	}

	// Load all fluids.

	// Air
	static_assert(AIR_FLUID_ID == 0);
	fluidsMap_[AIR_FLUID_NAME] = AIR_FLUID_ID;
	fluids_.emplace_back(AIR_FLUID_ID, AIR_FLUID_NAME, Fluid::Builder{
		.name = "",
		.fg = Cygnet::Color{0, 0, 0, 0},
		.density = 0,
	});
	builder.addFluid(fluids_.back().id, fluids_.back().fg, fluids_.back().bg);

	// Solid
	static_assert(SOLID_FLUID_ID == 1);
	fluidsMap_[SOLID_FLUID_NAME] = SOLID_FLUID_ID;
	fluids_.emplace_back(SOLID_FLUID_ID, SOLID_FLUID_NAME, Fluid::Builder{
		.name = "",
		.fg = Cygnet::Color{0, 0, 0, 0},
		.density = std::numeric_limits<float>::infinity(),
	});
	builder.addFluid(fluids_.back().id, fluids_.back().fg, fluids_.back().bg);

	// Fluids from mods
	for (auto &mod: mods_) {
		for (auto &fluidBuilder: mod.mod_->fluids_) {
			std::string fluidName = cat(mod.name(), "::", fluidBuilder.name);

			if (fluids_.size() >= INVALID_FLUID_ID) {
				warn << "Can't load fluid " << fluidName << ": Fluid overflow";
				continue;
			}

			Fluid::ID id = Fluid::ID(fluids_.size());
			fluidsMap_[fluidName] = id;
			fluids_.emplace_back(id, std::move(fluidName), fluidBuilder);
			builder.addFluid(fluids_.back().id, fluids_.back().fg, fluids_.back().bg);
		}
	}

	// Invalid
	fluidsMap_[INVALID_FLUID_NAME] = INVALID_FLUID_ID;
	while (fluids_.size() <= INVALID_FLUID_ID) {
		fluids_.emplace_back(fluids_.size(), INVALID_FLUID_NAME, Fluid::Builder{
			.name = "",
			.fg = Cygnet::Color{1, 0.19, 0.97, 1},
			.density = 0,
		});
		builder.addFluid(fluids_.back().id, fluids_.back().fg, fluids_.back().bg);
	}

	// Load recipe kinds.
	for (auto &mod: mods_) {
		for (auto &recipeType: mod.mod_->recipeKinds_) {
			std::string name = cat(mod.name(), "::", recipeType);
			recipes_[std::move(name)] = {};
		}
	}

	// Load recipes.
	std::vector<ItemStack> recipeInputs;
	for (auto &mod: mods_) {
		for (auto &recipeBuilder: mod.mod_->recipes_) {
			auto vec = recipes_.find(recipeBuilder.kind);
			if (vec == recipes_.end()) {
				warn << "Unknown recipe kind: " << recipeBuilder.kind;
				continue;
			}

			recipeInputs.clear();
			for (const auto &inputBuilder: recipeBuilder.inputs) {
				recipeInputs.push_back({
					&getItem(inputBuilder.item),
					inputBuilder.count,
				});
			}

			ItemStack output;
			if (recipeBuilder.output.count != 0) {
				output = {
					&getItem(recipeBuilder.output.item),
					recipeBuilder.output.count,
				};
			}
			vec->second.push_back({
				.inputs = recipeInputs, // Copy, don't move, so we shrink to fit
				.output = output,
				.kind = recipeBuilder.kind,
			});
		}
	}

	// Fix up tiles.
	for (auto &mod: mods_) {
		for (auto &tileBuilder: mod.mod_->tiles_) {
			std::string name = cat(mod.name(), "::", tileBuilder.name);
			Tile &tile = tiles_[tilesMap_[name]];
			if (tileBuilder.droppedItem) {
				tile.more->droppedItem = &getItem(tileBuilder.droppedItem.value());
			}
		}
	}

	// Load world gens and entities.
	for (auto &mod: mods_) {
		for (auto &worldGenFactory: mod.mod_->worldGens_) {
			worldGenFactories_.emplace(worldGenFactory.name, worldGenFactory);
		}

		for (auto &entCollFactory: mod.mod_->entities_) {
			entCollFactories_.emplace(entCollFactory.name, entCollFactory);
		}
	}

	// Load actions.
	for (auto &mod: mods_) {
		for (auto action: mod.mod_->actions_) {
			action.name = cat(mod.name(), "::", action.name);
			actions_.push_back(std::move(action));
		}
	}

	invalidItem_ = &items_[INVALID_TILE_ID];
	resources_ = Cygnet::ResourceManager(std::move(builder));
}

World::World(
		Game *game,
		uint32_t seed,
		std::span<const std::string> modPaths):
	game_(game),
	seed_(seed)
{
	mods_ = loadMods(modPaths);
	buildResources();
}

World::~World()
{
	// All the datastructures which get filled when loading mods must have been
	// constructed before the mods are loaded.
	// However, those datastructures must be destroyed *before* unloading the mods,
	// because the datastructures will contain owning pointers to objects which
	// were instantiated by the mods, and where the destructor functions are.
	// provided by the mods.
	// Unloading the mods before destructing the objects causes a segfault.
	tiles_.clear();
	tilesMap_.clear();
	items_.clear();
	recipes_.clear();
	worldGenFactories_.clear();
	entCollFactories_.clear();
}

/*
float World::findImageYOffset(ImageAsset &image)
{
	int y;
	bool done = false;
	for (y = 0; y < TILE_SIZE && !done; ++y) {
		unsigned char *row = &image.data[(TILE_SIZE - y - 1) * TILE_SIZE * 4];
		for (int x = 0; x < TILE_SIZE; ++x) {
			unsigned char *pix = &row[x * 4];
			if (pix[3] > 0) {
				done = true;
				break;
			}
		}
	}

	return (y - 1) / float(TILE_SIZE);
}
*/

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos)
{
	ZoneScopedN("World::ChunkRenderer tick");
	int l = 0;

	RTClock clock;
	for (int i = 0; i < 4; ++i) {
		chunkLine(l, plane, abspos, Vec2i(0, -1));
		chunkLine(l, plane, abspos, Vec2i(1, 0));
		l += 1;
		chunkLine(l, plane, abspos, Vec2i(0, 1));
		chunkLine(l, plane, abspos, Vec2i(-1, 0));
		l += 1;
	}
}

void World::setWorldGen(std::string gen)
{
	defaultWorldGen_ = std::move(gen);
}

void World::spawnPlayer()
{
	playerRef_ = planes_[currentPlane_].plane->spawnPlayer();
	player_ = playerRef_.trait<BodyTrait>();
}

void World::setCurrentPlane(WorldPlane &plane)
{
	currentPlane_ = plane.id_;
}

WorldPlane &World::addPlane(std::string gen)
{
	WorldPlane::ID id = planes_.size();
	auto it = worldGenFactories_.find(gen);

	if (it == worldGenFactories_.end()) {
		panic << "Tried to add plane with non-existent world gen " << gen << "!";
		abort();
	}

	std::vector<std::unique_ptr<EntityCollection>> colls;
	colls.reserve(entCollFactories_.size());
	for (auto &fact: entCollFactories_) {
		colls.emplace_back(fact.second.create(fact.second.name));
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(*this, seed_);
	planes_.push_back({
		.worldGen = std::move(gen),
		.plane = std::make_unique<WorldPlane>(
			id, this, std::move(g), std::move(colls)),
	});
	return *planes_[id].plane;
}

Tile::ID World::getTileID(std::string_view name)
{
	auto iter = tilesMap_.find(name);

	if (iter == tilesMap_.end()) {
		warn << "Tried to get non-existent tile " << name << "!";
		return INVALID_TILE_ID;
	}

	return iter->second;
}

Item &World::getItem(std::string_view name)
{
	auto iter = itemsMap_.find(name);

	if (iter == itemsMap_.end()) {
		warn << "Tried to get non-existent item " << name << "!";
		return invalidItem();
	}

	return items_[iter->second];
}

Fluid::ID World::getFluidID(std::string_view name)
{
	auto it = fluidsMap_.find(name);

	if (it == fluidsMap_.end()) {
		warn << "Tried to get non-existent fluid " << name << "!";
		return INVALID_FLUID_ID;
	}

	return it->second;
}

Cygnet::RenderSprite &World::getSprite(std::string_view name)
{
	auto iter = sprites_.find(name);

	if (iter == sprites_.end()) {
		warn << "Tried to get non-existent sprite " << name << "!";
		return sprites_.at(INVALID_SPRITE_NAME);
	}

	return iter->second;
}

SoundAsset *World::getSound(std::string_view name)
{
	auto iter = sounds_.find(name);

	if (iter == sounds_.end()) {
		warn << "Tried to get non-existent sound " << name << "!";
		return &sounds_.at(INVALID_SOUND_NAME);
	}

	return &iter->second;
}

Cygnet::Color World::backgroundColor()
{
	return planes_[currentPlane_].plane->backgroundColor();
}

void World::draw(Cygnet::Renderer &rnd)
{
	ZoneScopedN("World draw");
	planes_[currentPlane_].plane->draw(rnd);
}

void World::update(float dt)
{
	ZoneScopedN("World update");
	for (auto &plane: planes_) {
		plane.plane->update(dt);
	}

	game_->cam_.pos = player_->pos + player_->size / 2;
}

bool World::tick(float dt, RTDeadline deadline)
{
	ZoneScopedN("World tick");

	if (!tickProgress_.ongoing) {
		chunkRenderer_.tick(
			*planes_[currentPlane_].plane,
			ChunkPos((int)player_->pos.x / CHUNK_WIDTH, (int)player_->pos.y / CHUNK_HEIGHT));

		resourceTickCounter_ += 1;
		if (resourceTickCounter_ >= 2) {
			resources_.tick();
			resourceTickCounter_ = 0;
		}
	}

	bool allPlanesTicked = true;
	for (auto &plane: planes_) {
		if (tickProgress_.tickedPlanes.contains(plane.plane->id_)) {
			continue;
		}

		if (!plane.plane->tick(dt, deadline)) {
			allPlanesTicked = false;
			break;
		}

		tickProgress_.tickedPlanes.insert(plane.plane->id_);
	}

	if (allPlanesTicked) {
		tickProgress_.tickedPlanes.clear();
		tickProgress_.ongoing = false;
		return true;
	} else {
		tickProgress_.ongoing = true;
		return false;
	}
}

void World::serialize(proto::World::Builder w)
{
	auto tilesBuilder = w.initTiles(tiles_.size());
	for (size_t i = 0; i < tiles_.size(); ++i) {
		tilesBuilder.set(i, tiles_[i].name.string());
	}

	auto planesBuilder = w.initPlanes(planes_.size());
	for (size_t i = 0; i < planes_.size(); ++i) {
		planes_[i].plane->serialize(planesBuilder[i]);
		planesBuilder[i].setWorldGen(planes_[i].worldGen);
	}

	playerRef_.serialize(w.initPlayer());
	w.setCurrentPlane(currentPlane_);
	w.setSeed(seed_);
}

void World::deserialize(proto::World::Reader r)
{
	std::vector<Tile::ID> tileMap;
	auto tiles = r.getTiles();
	tileMap.reserve(tiles.size());
	for (auto tile: tiles) {
		tileMap.push_back(getTileID(tile.cStr()));
	}

	// Seed must exist before we deserialize planes
	seed_ = r.getSeed();

	auto planes = r.getPlanes();
	planes_.clear();
	planes_.reserve(planes.size());
	for (auto plane: planes) {
		addPlane(plane.getWorldGen().cStr()).deserialize(plane, tileMap);
	}

	currentPlane_ = r.getCurrentPlane();
	playerRef_.deserialize(currentPlane().getContext(), r.getPlayer());
	player_ = playerRef_.trait<BodyTrait>();
	if (!player_) {
		panic << "Missing player body!";
		throw std::runtime_error("Missing player body");
	}
}

}
