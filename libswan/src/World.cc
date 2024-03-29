#include "World.h"

#include "log.h"
#include "Game.h"
#include "Clock.h"
#include "assets.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

namespace Swan {

static void chunkLine(int l, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir)
{
	for (int i = 0; i < l; ++i) {
		plane.slowGetChunk(abspos).keepActive();
		abspos += dir;
	}
}

std::vector<ModWrapper> World::loadMods(std::vector<std::string> paths)
{
	std::vector<ModWrapper> mods;

	mods.reserve(paths.size());

	for (auto &path: paths) {
		OS::Dynlib dl(path + "/mod");
		auto create = dl.get<Mod *(*)()>("mod_create");
		if (create == NULL) {
			warn << path << ": No 'mod_create' function!";
			continue;
		}

		std::unique_ptr<Mod> mod(create());
		mods.push_back(ModWrapper(std::move(mod), std::move(path), std::move(dl)));
	}

	return mods;
}

void World::buildResources()
{
	Cygnet::ResourceBuilder builder(&game_->renderer_);

	auto fillTileImage = [&](unsigned char *data, int r, int g, int b, int a) {
		for (size_t i = 0; i < TILE_SIZE * TILE_SIZE; ++i) {
			data[i * 4 + 0] = r;
			data[i * 4 + 1] = g;
			data[i * 4 + 2] = b;
			data[i * 4 + 2] = a;
		}
	};

	// Assets are namespaced on the mod, so if something references, say,
	// "core::stone", we need to know which directory the "core" mod is in
	for (auto &mod: mods_) {
		modPaths_[mod.name()] = mod.path_;
	}

	modPaths_["@"] = ".";

	// Load built-in sounds.
	sounds_[INVALID_SOUND_NAME] = SoundAsset{};
	if (auto thud = loadSoundAsset(modPaths_, "@::sounds/thud")) {
		sounds_[THUD_SOUND_NAME] = std::move(thud.value());
	}
	else {
		warn
			<< "Failed to load built-in sound " << THUD_SOUND_NAME
			<< ": " << thud.err();
		sounds_[THUD_SOUND_NAME] = SoundAsset{};
	}

	// Load sounds.
	for (auto &mod: mods_) {
		for (auto soundPath: mod.sounds()) {
			std::string name = cat(mod.name(), "::", soundPath);
			auto soundResult = loadSoundAsset(modPaths_, name);
			SoundAsset sound;

			if (soundResult) {
				sounds_[name] = std::move(soundResult.value());
			}
			else {
				warn << '\'' << name << ": " << soundResult.err();
				sounds_[name] = SoundAsset{};
			}
		}
	}

	// After this point, 'sounds_' *must* be unchanged.
	// We rely on pointers to be stable from now on.
	SoundAsset *fallbackSound = &sounds_[INVALID_SOUND_NAME];
	SoundAsset *thudSound = &sounds_[THUD_SOUND_NAME];

	struct ImageAsset fallbackImage = {
		.width = 32,
		.frameHeight = 32,
		.frameCount = 1,
		.repeatFrom = 0,
		.data = std::make_unique<unsigned char[]>(TILE_SIZE * TILE_SIZE * 4),
	};

	fillTileImage(fallbackImage.data.get(),
		PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE, 255);

	auto airImage = std::make_unique<unsigned char[]>(TILE_SIZE * TILE_SIZE * 4);
	fillTileImage(airImage.get(),
		PLACEHOLDER_RED, PLACEHOLDER_GREEN, PLACEHOLDER_BLUE, 255);

	// Let tile ID 0 be the invalid tile
	builder.addTile(INVALID_TILE_ID, fallbackImage.data.get());
	tilesMap_[INVALID_TILE_NAME] = INVALID_TILE_ID;
	tiles_.push_back(Tile(INVALID_TILE_ID, INVALID_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
		.isSolid = false,
	}));
	items_.emplace(INVALID_TILE_NAME, Item(INVALID_TILE_ID, INVALID_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
	}));

	// ...And tile ID 1 be the air tile
	builder.addTile(AIR_TILE_ID, std::move(airImage));
	tilesMap_[AIR_TILE_NAME] = AIR_TILE_ID;
	tiles_.push_back(Tile(AIR_TILE_ID, AIR_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
		.isSolid = false,
	}));
	items_.emplace(AIR_TILE_NAME, Item(AIR_TILE_ID, AIR_TILE_NAME, {
		.name = "", .image = "", // Not used in this case
	}));

	// Set sounds for all built-in tiles
	for (auto &tile: tiles_) {
		tile.stepSounds[0] = fallbackSound;
		tile.stepSounds[1] = fallbackSound;
		tile.breakSound = fallbackSound;
	}

	auto loadTileImage = [&](std::string path) -> Result<ImageAsset> {
		// Don't force all tiles/items to have an associated image.
		// It could be that some tiles/items exist for a purpose which implies
		// it should never actually be visible.
		if (path == INVALID_TILE_NAME) {
			ImageAsset asset{
				.width = 32,
				.frameHeight = 32,
				.frameCount = 1,
				.repeatFrom = 0,
				.data = std::make_unique<unsigned char[]>(TILE_SIZE * TILE_SIZE * 4),
			};
			memcpy(asset.data.get(), fallbackImage.data.get(), TILE_SIZE * TILE_SIZE * 4);
			return {Ok, std::move(asset)};
		}

		auto image = loadImageAsset(modPaths_, path);
		if (!image) {
			warn << '\'' << path << "': " << image.err();
			return {Err, cat("'", path, "': ", image.err())};
		}
		else if (image->width != TILE_SIZE) {
			warn << '\'' << path << "': Width must be " << TILE_SIZE << " pixels";
			return {Err, cat("'", path, "': Width must be ", std::to_string(TILE_SIZE), " pixels")};
		}
		else {
			return image;
		}
	};

	// Need to fill in every tile before we do items,
	// because all items will end up after all tiles in the tile atlas.
	// In the rendering system, there's no real difference between a tile
	// and an item.
	for (auto &mod: mods_) {
		for (auto &tileBuilder: mod.tiles()) {
			auto image = loadTileImage(tileBuilder.image);

			std::string tileName = cat(mod.name(), "::", tileBuilder.name);
			Tile::ID tileId = tiles_.size();

			if (image) {
				builder.addTile(tileId, std::move(image->data));
			}
			else {
				warn << image.err();
				builder.addTile(tileId, fallbackImage.data.get());
			}

			tilesMap_[tileName] = tileId;
			tiles_.push_back(Tile(tileId, tileName, std::move(tileBuilder)));
			auto &tile = tiles_.back();

			if (tileBuilder.breakSound) {
				tile.breakSound = getSound(tileBuilder.breakSound.value());
			}
			else {
				tile.breakSound = thudSound;
			}

			if (tileBuilder.stepSound) {
				auto &s = tileBuilder.stepSound.value();
				tile.stepSounds[0] = getSound(cat(s, "1"));
				tile.stepSounds[1] = getSound(cat(s, "2"));
			}
			else if (tile.isSolid) {
				tile.stepSounds[0] = thudSound;
				tile.stepSounds[1] = thudSound;
			}
			else {
				tile.stepSounds[0] = fallbackSound;
				tile.stepSounds[1] = fallbackSound;
			}
		}
	}

	// Create a dummy item for each tile.
	for (auto &tile: tiles_) {
		items_.emplace(tile.name, Item(tile.id, tile.name, {}));
		items_.at(tile.name).tile = &tile;
	}

	// Put all items after all the tiles
	Tile::ID nextItemId = tiles_.size();

	// Load all items which aren't just tiles in disguise.
	for (auto &mod: mods_) {
		for (auto &itemBuilder: mod.items()) {
			auto image = loadTileImage(itemBuilder.image);

			std::string itemName = cat(mod.name(), "::", itemBuilder.name);
			Tile::ID itemId = nextItemId++;

			if (image) {
				builder.addTile(itemId, std::move(image->data));
			}
			else {
				warn << image.err();
				builder.addTile(itemId, fallbackImage.data.get());
			}

			items_.erase(itemName);
			items_.emplace(itemName, Item(itemId, itemName, itemBuilder));
			if (itemBuilder.tile) {
				items_.at(itemName).tile = &getTile(itemBuilder.tile.value());
			}
		}
	}

	// Load recipes.
	std::vector<Recipe::Items> recipeInputs;
	for (auto &mod: mods_) {
		for (auto &recipeBuilder: mod.recipes()) {
			recipeInputs.clear();
			for (const auto &inputBuilder: recipeBuilder.inputs) {
				recipeInputs.push_back({
					.count = inputBuilder.count,
					.item = &getItem(inputBuilder.item),
				});
			}

			recipes_.push_back({
				.inputs = recipeInputs, // Copy, don't move, so we shrink to fit
				.output = {
					.count = recipeBuilder.output.count,
					.item = &getItem(recipeBuilder.output.item),
				},
				.kind = recipeBuilder.kind,
			});
		}
	}

	// Load built-in sprites.
	builder.addSprite(INVALID_SPRITE_NAME, fallbackImage.data.get(), {
		.width = fallbackImage.width,
		.height = fallbackImage.frameHeight * fallbackImage.frameCount,
		.frameHeight = fallbackImage.frameHeight,
		.repeatFrom = fallbackImage.repeatFrom,
	});

	// Load sprites.
	for (auto &mod: mods_) {
		for (auto spritePath: mod.sprites()) {
			std::string name = cat(mod.name(), "::", spritePath);
			auto imageResult = loadImageAsset(modPaths_, name);
			ImageAsset *image;

			if (imageResult) {
				image = &imageResult.value();
			}
			else {
				warn << '\'' << name << "': " << imageResult.err();
				image = &fallbackImage;
			}

			builder.addSprite(name, image->data.get(), {
				.width = image->width,
				.height = image->frameHeight * image->frameCount,
				.frameHeight = image->frameHeight,
				.repeatFrom = image->repeatFrom,
			});
		}
	}

	// Fix up tiles.
	for (auto &mod: mods_) {
		for (auto &tileBuilder: mod.tiles()) {
			std::string name = cat(mod.name(), "::", tileBuilder.name);
			Tile &tile = tiles_[tilesMap_[name]];
			if (tileBuilder.droppedItem) {
				tile.droppedItem = &getItem(tileBuilder.droppedItem.value());
			}
		}
	}

	// Load world gens and entities.
	for (auto &mod: mods_) {
		for (auto &worldGenFactory: mod.worldGens()) {
			std::string name = cat(mod.name(), "::", worldGenFactory.name);
			worldGenFactories_.emplace(name, worldGenFactory);
		}

		for (auto &entCollFactory: mod.entities()) {
			std::string name = cat(mod.name(), "::", entCollFactory.name);
			entCollFactories_.emplace(name, entCollFactory);
		}
	}

	resources_ = Cygnet::ResourceManager(std::move(builder));
}

World::World(Game *game, unsigned long randSeed, std::vector<std::string> modPaths):
	game_(game), random_(randSeed)
{
	mods_ = loadMods(std::move(modPaths));
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
	playerRef_ = planes_[currentPlane_]->spawnPlayer();
	player_ = playerRef_.trait<BodyTrait>();
}

void World::setCurrentPlane(WorldPlane &plane)
{
	currentPlane_ = plane.id_;
}

WorldPlane &World::addPlane(const std::string &gen)
{
	WorldPlane::ID id = planes_.size();
	auto it = worldGenFactories_.find(gen);

	if (it == worldGenFactories_.end()) {
		panic << "Tried to add plane with non-existent world gen " << gen << "!";
		abort();
	}

	std::vector<std::unique_ptr<EntityCollection> > colls;
	colls.reserve(entCollFactories_.size());
	for (auto &fact: entCollFactories_) {
		colls.emplace_back(fact.second.create(fact.second.name));
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(*this);
	planes_.push_back(std::make_unique<WorldPlane>(
		id, this, std::move(g), std::move(colls)));
	return *planes_[id];
}

Tile::ID World::getTileID(const std::string &name)
{
	auto iter = tilesMap_.find(name);

	if (iter == tilesMap_.end()) {
		warn << "Tried to get non-existent tile " << name << "!";
		return INVALID_TILE_ID;
	}

	return iter->second;
}

Tile &World::getTile(const std::string &name)
{
	Tile::ID id = getTileID(name);

	return getTileByID(id);
}

Item &World::getItem(const std::string &name)
{
	auto iter = items_.find(name);

	if (iter == items_.end()) {
		return items_.at(INVALID_TILE_NAME);
	}

	return iter->second;
}

Cygnet::RenderSprite &World::getSprite(const std::string &name)
{
	auto iter = resources_.sprites_.find(name);

	if (iter == resources_.sprites_.end()) {
		warn << "Tried to get non-existent sprite " << name << "!";
		return resources_.sprites_.at(INVALID_SPRITE_NAME);
	}

	return iter->second;
}

SoundAsset *World::getSound(const std::string &name)
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
	return planes_[currentPlane_]->backgroundColor();
}

void World::draw(Cygnet::Renderer &rnd)
{
	ZoneScopedN("World draw");
	planes_[currentPlane_]->draw(rnd);
}

void World::ui()
{
	ZoneScopedN("World UI");
	planes_[currentPlane_]->ui();
}

void World::update(float dt)
{
	ZoneScopedN("World update");
	for (auto &plane: planes_) {
		plane->update(dt);
	}

	game_->cam_.pos = player_->pos + player_->size / 2;
}

void World::tick(float dt)
{
	ZoneScopedN("World tick");
	for (auto &plane: planes_) {
		plane->tick(dt);
	}

	chunkRenderer_.tick(
		*planes_[currentPlane_],
		ChunkPos((int)player_->pos.x / CHUNK_WIDTH, (int)player_->pos.y / CHUNK_HEIGHT));
}

}
