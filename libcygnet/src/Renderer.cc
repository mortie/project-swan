#include "Renderer.h"

#include "shaders.h"
#include "Program.h"
#include "util.h"

namespace Cygnet {

struct TexturedQuad: public GlProgram {
	using GlProgram::GlProgram;

	GlLoc position = attribLoc("position");
	GlLoc texCoord = attribLoc("texCoord");
	GlLoc tex = uniformLoc("tex");
};

struct ColoredQuad: public GlProgram {
	using GlProgram::GlProgram;

	GlLoc position = attribLoc("position");
	GlLoc color = uniformLoc("color");
};

struct RendererState {
	GlVxShader texturedQuadVx{Shaders::texturedQuadVx};
	GlFrShader texturedFr{Shaders::texturedFr};
	GlVxShader coloredQuadVx{Shaders::texturedQuadVx};
	GlFrShader coloredFr{Shaders::texturedFr};

	TexturedQuad texturedQuad{texturedQuadVx, texturedFr};
	ColoredQuad coloredQuad{coloredQuadVx, coloredFr};
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {}

Renderer::~Renderer() = default;

}
