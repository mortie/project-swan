#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Fluid.h"
#include "Tile.h"
#include "Item.h"
#include "Recipe.h"
#include "WorldGen.h"
#include "EntityCollection.h"
#include "OS.h"

namespace cpptoml {
class table;
}

namespace Swan {

class ModWrapper;

class Mod {
public:
	Mod(std::string name): name_(std::move(name))
	{}
	virtual ~Mod() = default;

	void registerTile(Tile::Builder &&tile);
	void registerItem(Item::Builder &&item);
	void registerFluid(Fluid::Builder &&item);
	void registerRecipe(Recipe::Builder &&recipe);
	void registerSprite(std::string &&sprite);
	void registerSound(std::string &&sprite);
	void registerRecipeKind(std::string &&kind);

	void registerStepSounds(std::string &&sprite)
	{
		registerSound(sprite + "1");
		registerSound(sprite + "2");
	}

	template<typename WG>
	void registerWorldGen(std::string name);

	template<typename Ent>
	void registerEntity(const std::string name);

	virtual void start(World &)
	{}

private:
	const std::string name_;
	std::vector<Tile::Builder> tiles_;
	std::vector<Item::Builder> items_;
	std::vector<Fluid::Builder> fluids_;
	std::vector<Recipe::Builder> recipes_;
	std::vector<std::string> sprites_;
	std::vector<std::string> sounds_;
	std::vector<WorldGen::Factory> worldGens_;
	std::vector<EntityCollection::Factory> entities_;
	std::vector<std::string> recipeKinds_;

	friend ModWrapper;
};

class ModWrapper {
public:
	ModWrapper(std::unique_ptr<Mod> mod, std::string path, OS::Dynlib lib);
	ModWrapper(ModWrapper &&other) noexcept;
	~ModWrapper();

	const std::string &name()
	{
		return mod_->name_;
	}

	const std::vector<Tile::Builder> &tiles()
	{
		return mod_->tiles_;
	}

	const std::vector<Item::Builder> &items()
	{
		return mod_->items_;
	}

	const std::vector<Fluid::Builder> &fluids()
	{
		return mod_->fluids_;
	}

	const std::vector<Recipe::Builder> &recipes()
	{
		return mod_->recipes_;
	}

	const std::vector<std::string> &sprites()
	{
		return mod_->sprites_;
	}

	const std::vector<std::string> &sounds()
	{
		return mod_->sounds_;
	}

	const std::vector<std::string> &recipeKinds()
	{
		return mod_->recipeKinds_;
	}

	const std::vector<WorldGen::Factory> &worldGens()
	{
		return mod_->worldGens_;
	}

	const std::vector<EntityCollection::Factory> &entities()
	{
		return mod_->entities_;
	}

	// This uses string& due to cpptoml >_>
	std::string lang(const std::string &cat, const std::string &name);

	void loadLang(std::string_view lang);

	std::unique_ptr<Mod> mod_;
	std::string path_;
	OS::Dynlib dynlib_;

	std::shared_ptr<cpptoml::table> lang_;
};

}
