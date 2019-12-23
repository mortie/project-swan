#pragma once

#include <string>

#include "Resource.h"

namespace Swan {

// TODO: Switch to struct
class Item {
public:
	struct Builder {
		std::string name;
		std::string image;
	};

	Item(const ResourceManager &resources, const Builder &builder):
		name_(builder.name), image_(resources.getImage(builder.image)) {}

	const std::string name_;
	const ImageResource &image_;

	static std::unique_ptr<Item> createInvalid(Context &ctx);
};

}
