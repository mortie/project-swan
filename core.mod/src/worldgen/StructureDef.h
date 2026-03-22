#pragma once

#include <swan/swan.h>
#include <span>

#include "WorldArea.h"

namespace CoreMod {

class StructureDef {
public:
	virtual void generateArea(WorldArea &area) = 0;

protected:
	~StructureDef() = default;
};

}
