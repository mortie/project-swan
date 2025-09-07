#include <cpptoml.h>
#include <fstream>
#include <optional>

#include <swan/util.h>
#include "Mod.h"
#include "EntityCollectionImpl.h"
#include "log.h"

namespace Swan {

void Mod::registerTile(Tile::Builder tile)
{
	tiles_.push_back(std::move(tile));
}

void Mod::registerItem(Item::Builder item)
{
	items_.push_back(std::move(item));
}

void Mod::registerFluid(Fluid::Builder fluid)
{
	fluids_.push_back(std::move(fluid));
}

void Mod::registerRecipe(Recipe::Builder recipe)
{
	recipes_.push_back(std::move(recipe));
}

void Mod::registerRecipeKind(std::string kind)
{
	recipeKinds_.push_back(std::move(kind));
}

ModWrapper::ModWrapper(std::unique_ptr<Mod> mod, std::string path, OS::Dynlib lib):
	mod_(std::move(mod)), path_(std::move(path)), dynlib_(std::move(lib))
{
	lang_ = cpptoml::make_table();
	loadLang("en");
}

ModWrapper::ModWrapper(ModWrapper &&other) noexcept = default;

ModWrapper::~ModWrapper()
{
	// Mod::~Mod will destroy stuff allocated by the dynlib,
	// so we must run its destructor before deleting the dynlib
	mod_.reset();
}

std::string ModWrapper::lang(const std::string &cat, const std::string &name)
{
	auto catTable = lang_->get_table(cat);
	if (!catTable) {
		warn << "Failed to find lang table for " << cat << '/' << name;
		return name;
	}

	auto sep = name.find("::");
	cpptoml::option<std::string> lang;
	if (sep == std::string::npos) {
		lang = catTable->get_as<std::string>(name);
	}
	else {
		auto mainPart = name.substr(0, sep);
		lang = catTable->get_as<std::string>(mainPart);
	}

	if (!lang) {
		warn << "Failed to find lang item for " << cat << '/' << name;
		return name;
	}

	return *lang;
}

void ModWrapper::loadLang(std::string_view lang)
{
	std::string path = cat(path_, "/assets/lang/", lang, ".toml");
	std::ifstream langFile(path);
	if (!langFile) {
		warn << "Failed to load " << lang << ": Couldn't open " << path;
		if (lang != "en") {
			loadLang("en");
		}
		return;
	}

	cpptoml::parser parser(langFile);
	try {
		lang_ = parser.parse();
	} catch (cpptoml::parse_exception &exc) {
		warn << "Failed to parse " << path << ": " << exc.what();
		if (lang != "en") {
			loadLang("en");
		}
		return;
	}
}

}
