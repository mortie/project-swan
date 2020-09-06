#pragma once

#include <vector>

#include "GlWrappers.h"

namespace Cygnet {

class RenderQueue {
public:
	struct Locs {
		GLint transform;
		GLint position;
		GLint texCoord;
		GLint tex;
	};

	RenderQueue(Locs locs, float scale):
			locs_(std::move(locs)), pixScale_(scale) {};
	RenderQueue(GlProgram &prog, float scale):
		RenderQueue({
			prog.uniformLoc("transform"),
			prog.attribLoc("position"),
			prog.attribLoc("texCoord"),
			prog.uniformLoc("tex"),
		}, scale) {}

	void show(float x, float y, GlTexture &tex) { show(x, y, tex.width(), tex.height(), tex.id()); }
	void show(float x, float y, float w, float h, GLuint tex) {
		queue_.push_back({ x, y, w * pixScale_, h * pixScale_, tex });
	}

	float pixScale() { return pixScale_; }
	void pixScale(float scale) { pixScale_ = scale; }

	float scaleX() { return mat_[0]; }
	void scaleX(float sx) { mat_[0] = sx; }
	float scaleY() { return -mat_[4]; }
	void scaleY(float sy) { mat_[4] = -sy; }
	float translateX() { return mat_[2]; }
	void translateX(float tx) { mat_[2] = tx; }
	float translateY() { return mat_[5]; }
	void translateY(float ty) { mat_[5] = ty; }

	void draw();

private:
	struct Entry {
		float x, y, w, h;
		GLuint tex;
	};

	Locs locs_;
	float pixScale_;
	GLfloat mat_[9] = {
		1,  0, 0,  // scaleX, 0,       translateX,
		0, -1, 0,  // 0,      -scaleY, translateY,
		0,  0, 1,  // 0,      0,       1
	};

	std::vector<Entry> queue_;
};

}
