#include "Renderer.h"

#include "shaders.h"
#include "Program.h"
#include "TileAtlas.h"
#include "util.h"

namespace Cygnet {

struct TexturedProg: public GlProgram {
	using GlProgram::GlProgram;

	GlLoc position = attribLoc("position");
	GlLoc texCoord = attribLoc("texCoord");
	GlLoc tex = uniformLoc("tex");
};

struct SolidColorProg: public GlProgram {
	using GlProgram::GlProgram;

	GlLoc position = attribLoc("position");
	GlLoc color = uniformLoc("color");
};

struct RendererState {
	GlVxShader basicVx{Shaders::basicVx};
	GlVxShader texturedVx{Shaders::texturedVx};
	GlFrShader solidColorFr{Shaders::solidColorFr};
	GlFrShader texturedFr{Shaders::texturedFr};

	TexturedProg texturedProg{texturedVx, texturedFr};
	SolidColorProg solidColorProg{basicVx, solidColorFr};

	TileAtlas atlas;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {}

Renderer::~Renderer() = default;

}
