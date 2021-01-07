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
		int maxStack = 64;
	};

	Item(const ResourceManager &resources, const Builder &builder):
		name_(builder.name), image_(resources.getImage(builder.image)),
		maxStack_(builder.maxStack) {}

	const std::string name_;
	const ImageResource &image_;
	const int maxStack_;

	static std::unique_ptr<Item> createInvalid(Context &ctx);

// For testing, we want to be able to create Items without an actual ImageResource.
// Tests can create a MockItem class which inherits from Item and uses this ctor,
// as long as the test never does anything which tries to follow the image_ member.
// Eventually, this should become unnecessary, because we don't need to require
// a complete ImageResource for a headless server, but for now, this will suffice.
protected:
	Item(const Builder &builder):
		name_(builder.name), image_(*(ImageResource *)this),
		maxStack_(builder.maxStack) {}
};

}
