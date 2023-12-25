#pragma once

#include <string>

#include "Item.h"

namespace Swan {

struct Recipe {
	struct Items {
		struct Builder {
			int count;
			std::string item;
		};

		int count;
		Item *item;
	};

	struct Builder {
		std::vector<Items::Builder> inputs;
		Items::Builder output;
		std::string kind;
	};

	std::vector<Items> inputs;
	Items output;
	std::string kind;
};

}
