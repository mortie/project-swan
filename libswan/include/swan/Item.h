#pragma once

#include <string>

#include "Resource.h"

namespace Swan {

class Item {
public:
	struct Builder {
		std::string name;
		std::string image;
	};

	Item(const ImageResource &image, const std::string &mod, const Builder &builder):
		name_(mod+"::"+builder.name), image_(image) {}

	const std::string name_;
	const ImageResource &image_;

	static std::unique_ptr<Item> createInvalid(Context &ctx);
};

}
