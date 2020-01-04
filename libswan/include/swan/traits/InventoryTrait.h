#pragma once

#include "../Vector2.h"

namespace Swan {

class ItemStack;

namespace InventoryTrait {

class Inventory;
class HasInventory {
public:
	virtual Inventory &getInventory() = 0;
};

class Inventory {
public:
};

}
}
