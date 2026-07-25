#pragma once

namespace Swan {

struct ContactDamage final {
	int damage = 1;
	float knockback = 10;
};

struct ContactDamageTrait {
	struct Tag {};

	virtual ContactDamage get(Tag) = 0;

protected:
	~ContactDamageTrait() = default;
};

}
