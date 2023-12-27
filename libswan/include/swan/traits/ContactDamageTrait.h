#pragma once

namespace Swan {

struct ContactDamageTrait {
	struct Tag {};

	struct Damage final {
		int damage = 1;
		float knockback = 10;
	};

	virtual Damage &get(Tag) = 0;

protected:
	~ContactDamageTrait() = default;
};

}
