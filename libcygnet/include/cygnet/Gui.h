#pragma once

#include <memory>
#include <string_view>

#include "Renderer.h"
#include "swan/Vector2.h"

namespace Cygnet {

class Gui {
public:
	Gui(Renderer *rnd);
	~Gui();

	Rect begin(Rect rect, Anchor anchor = Anchor::CENTER);
	void end();

	bool button(TextCache &cache, std::string_view text);

	void triggerActivate();
	void onMouseMove(Swan::Vec2 pos);
	void moveSelectionLeft();
	void moveSelectionRight();
	void moveSelectionUp();
	void moveSelectionDown();
	void endFrame();

private:
	struct Impl;

	std::unique_ptr<Impl> impl_;
};

}
