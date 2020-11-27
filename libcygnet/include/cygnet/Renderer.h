#pragma once

#include <memory>

namespace Cygnet {

struct RendererState;

class Renderer {
public:
	Renderer();
	~Renderer();

	void clear();
	void draw();

private:
	std::unique_ptr<RendererState> state_;
};

}
