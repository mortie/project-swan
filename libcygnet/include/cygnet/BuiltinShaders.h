#include <optional>

#include "GlWrappers.h"

namespace Cygnet {

struct BuiltinShaders {
	BuiltinShaders();

	GlShader textureVertex;
	GlShader textureFragment;
	GlShader whiteFragment;
};

}
