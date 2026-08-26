#include "WorldData.h"
#include "Clock.h"

namespace Swan {

WorldData::~WorldData()
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

Tile::ID WorldData::getTileID(std::string_view name)
{
	auto iter = tilesMap_.find(name);

	if (iter == tilesMap_.end()) {
		warn << "Tried to get non-existent tile " << name << "!";
		return INVALID_TILE_ID;
	}

	return iter->second;
}

Item &WorldData::getItem(std::string_view name)
{
	auto iter = itemsMap_.find(name);

	if (iter == itemsMap_.end()) {
		warn << "Tried to get non-existent item " << name << "!";
		return invalidItem();
	}

	return items_[iter->second];
}

Fluid::ID WorldData::getFluidID(std::string_view name)
{
	auto it = fluidsMap_.find(name);

	if (it == fluidsMap_.end()) {
		warn << "Tried to get non-existent fluid " << name << "!";
		return INVALID_FLUID_ID;
	}

	return it->second;
}

Cygnet::RenderSprite &WorldData::getSprite(std::string_view name)
{
	auto iter = sprites_.find(name);

	if (iter == sprites_.end()) {
		warn << "Tried to get non-existent sprite " << name << "!";
		return sprites_.at(INVALID_SPRITE_NAME);
	}

	return iter->second;
}

SoundAsset *WorldData::getSound(std::string_view name)
{
	auto iter = sounds_.find(name);

	if (iter == sounds_.end()) {
		warn << "Tried to get non-existent sound " << name << "!";
		return &sounds_.at(INVALID_SOUND_NAME);
	}

	return &iter->second;
}

void WorldData::loadMods(std::span<const std::string> paths)
{
	ScopedTimer timer("load mods");

	using T = decltype(mods_);
	mods_.~T();
	new (&mods_) T(paths.size());

	size_t i = 0;
	for (auto &path: paths) {
		OS::Dynlib dl(path + "/.swanbuild/mod");
		auto create = dl.get<ModCreateFn>("mod_create");
		if (create == NULL) {
			warn << path << ": No 'mod_create' function!";
			mods_.pop_back();
		}

		auto &w = mods_[i++];
		w.path_ = std::move(path);
		w.dynlib_ = std::move(dl);
		std::unique_ptr<Mod> mod(create(w));
		w.mod_ = std::move(mod);
		w.loadLang("en");
	}
}

void WorldData::buildResources(Cygnet::Renderer &rnd, std::vector<std::string> namesByID)
{
	ScopedTimer timer("build resources");

	Cygnet::ResourceBuilder builder(&rnd);

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

	/**
	 * Tiles and items
	 */

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

	// Build a map from string to ID, which we'll need later.
	HashMap<Tile::ID> idsByName;
	for (size_t i = 0; i < namesByID.size(); ++i) {
		idsByName[namesByID[i]] = i + 2;
	}
	Tile::ID nextID = namesByID.size() + 2;

	// Fill in all tiles before we do items.
	for (auto &mod: mods_) {
		for (auto &tileBuilder: mod.mod_->tiles_) {
			std::string tileName = cat(mod.name(), "::", tileBuilder.name);
			auto idIt = idsByName.find(tileName);
			Tile::ID tileId = idIt == idsByName.end() ? nextID++ : idIt->second;

			if (tiles_.size() <= tileId) {
				tiles_.resize(tileId + 1);
			}

			tilesMap_[tileName] = tileId;
			Tile &tile = tiles_[tileId] = Tile(tileId, tileName, std::move(tileBuilder));

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
			auto metaIt = tileMeta.find(tileBuilder.image);
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

			if (items_.size() <= tileId) {
				items_.resize(tileId + 1);
			}

			auto name = tile.name.string();
			itemsMap_[name] = tile.id;
			Item &item = items_[tileId] = Item(tile.id, name, {
				.name = "",
				.lightLevel = tile.more->lightLevel,
			});
			item.displayName = mod.lang("items", tileBuilder.name);
			item.yOffset = yOffset;

			// Tiles whose names contain '::' are "variants".
			// Convention is to have one tile without a '::' which represents
			// the "logical" tile, and then make variants with '::' in the name.
			item.hidden = tileBuilder.name.find("::") != std::string::npos;
		}
	}

	// We can now fix up the tile pointers in items.
	// All items we have so far will have an associated tile.
	for (auto &item: items_) {
		item.tile = &tiles_[item.id];
	}

	// Put all real items after all the tiles
	nextID = tiles_.size();

	// Load all items which aren't just tiles in disguise.
	for (auto &mod: mods_) {
		for (auto &itemBuilder: mod.mod_->items_) {
			std::string itemName = cat(mod.name(), "::", itemBuilder.name);
			auto idIt = idsByName.find(itemName);
			Tile::ID itemId = idIt == idsByName.end() ? nextID++ : idIt->second;

			if (items_.size() <= itemId) {
				items_.resize(itemId + 1);
			}

			auto metaIt = tileMeta.find(itemBuilder.image);

			float yOffset = 0;
			if (metaIt != tileMeta.end()) {
				yOffset = metaIt->second.yOffset;
			}
			builder.addTile(itemId, itemBuilder.image);

			items_[itemId] = Item(itemId, itemName, itemBuilder);
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

	// Prune old names from namesByID
	for (size_t i = 0; i < namesByID.size(); ++i) {
		auto &name = namesByID[i];
		if (!itemsMap_.contains(name) && !tilesMap_.contains(name)) {
			name = "";
		}
	}

	// Add anything we may be missing
	while (namesByID.size() < tiles_.size() - 2) {
		namesByID.push_back(tiles_[namesByID.size() + 2].name.string());
	}
	while (namesByID.size() < items_.size() - 2) {
		namesByID.push_back(items_[namesByID.size() + 2].name);
	}

	// Keep it around for later
	namesByID_ = std::move(namesByID);

	/**
	 * End tiles and items
	 */

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

	resources_ = Cygnet::ResourceManager(std::move(builder));
}

}
