#pragma once

#include "../common.h"

namespace Swan {

struct ContactDamageTrait {
	struct Tag{};

	struct Damage final {
		int damage = 1;
		float knockback = 10;
	};

	virtual Damage &get(Tag) = 0;
};

}
