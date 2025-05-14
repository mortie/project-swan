#pragma once

#include <string>
#include <vector>

#include "ItemStack.h"

namespace Swan {

struct Recipe {
	struct Builder {
		struct Items {
			int count = 0;
			std::string item = "";
		};

		std::vector<Items> inputs;
		Items output;
		std::string kind;
	};

	std::vector<ItemStack> inputs;
	ItemStack output;
	std::string kind;
};

}
