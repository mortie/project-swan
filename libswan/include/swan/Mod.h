#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Action.h"
#include "Fluid.h"
#include "Tile.h"
#include "Item.h"
#include "Recipe.h"
#include "WorldGen.h"
#include "EntityCollection.h"
#include "OS.h"

namespace cpptomlng {
class table;
}

namespace Swan {

class World;
class ModWrapper;

class Mod {
public:
	Mod(std::string name): name_(std::move(name))
	{}
	virtual ~Mod() = default;

	void registerTile(Tile::Builder tile)
	{
		tiles_.push_back(std::move(tile));
	}

	void registerItem(Item::Builder item)
	{
		items_.push_back(std::move(item));
	}

	void registerFluid(Fluid::Builder item)
	{
		fluids_.push_back(std::move(item));
	}

	void registerRecipe(Recipe::Builder recipe)
	{
		recipes_.push_back(std::move(recipe));
	}

	void registerRecipeKind(std::string kind)
	{
		recipeKinds_.push_back(std::move(kind));
	}

	void registerAction(ActionSpec action)
	{
		actions_.push_back(std::move(action));
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
	std::vector<WorldGen::Factory> worldGens_;
	std::vector<EntityCollection::Factory> entities_;
	std::vector<std::string> recipeKinds_;
	std::vector<ActionSpec> actions_;

	friend ModWrapper;
	friend World;
};

class ModWrapper {
public:
	ModWrapper(std::unique_ptr<Mod> mod, std::string path, OS::Dynlib lib);
	ModWrapper(ModWrapper &&other) noexcept;
	~ModWrapper();

	std::string_view name() { return mod_->name_; }

	// This uses string& due to cpptoml >_>
	std::string lang(const std::string &cat, const std::string &name);

	void loadLang(std::string_view lang);

	std::unique_ptr<Mod> mod_;
	std::string path_;
	OS::Dynlib dynlib_;

	std::shared_ptr<cpptomlng::table> lang_;
};

}
