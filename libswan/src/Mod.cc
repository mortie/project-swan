#include <cpptoml.h>
#include <fstream>

#include <swan/util.h>
#include <swan/log.h>
#include "Mod.h"

namespace Swan {

std::optional<ModInfo> ModInfo::parse(std::string path)
{
	std::string modTomlPath = cat(path, "/mod.toml");
	std::ifstream f(modTomlPath);
	if (!f) {
		warn << "Failed to parse " << modTomlPath << ": File doesn't exist";
		return std::nullopt;
	}

	cpptoml::parser parser(f);
	std::shared_ptr<cpptoml::table> root;
	try {
		root = parser.parse();
	} catch (cpptoml::parse_exception &exc) {
		warn << "Failed to parse " << modTomlPath << ": " << exc.what();
		return std::nullopt;
	}

	auto name = root->get("name");
	if (!name || !name->as<std::string>()) {
		warn << "Failed to parse " << modTomlPath << ": Missing 'name'";
		return std::nullopt;
	}

	auto version = root->get("version");
	if (!version || !version->as<std::string>()) {
		warn << "Failed to parse " << modTomlPath << ": Missing 'version'";
		return std::nullopt;
	}

	return ModInfo {
		.path = std::move(path),
		.name = std::move(name->as<std::string>()->get()),
		.version = std::move(version->as<std::string>()->get()),
	};
}

std::shared_ptr<cpptomlng::table> Mod::loadToml(std::string_view name)
{
	std::string path = cat(wrapper_.path_, "/assets/resources/", name, ".toml");

	std::ifstream langFile(path);
	if (!langFile) {
		warn << "Failed to open toml: " << path;
		return cpptomlng::make_table();
	}

	cpptoml::parser parser(langFile);
	try {
		return parser.parse();
	} catch (cpptoml::parse_exception &exc) {
		warn << "Failed to parse " << path << ": " << exc.what();
		return cpptomlng::make_table();
	}
}

ModWrapper::ModWrapper():
	lang_(cpptoml::make_table())
{}

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
