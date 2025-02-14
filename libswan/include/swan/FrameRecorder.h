#pragma once

#include <swan/Vector2.h>
#include <memory>
#include <cygnet/util.h>

namespace Swan {

class FrameRecorder {
public:
	FrameRecorder();
	~FrameRecorder();

	static bool isAvailable();

	bool begin(int w, int h, int fps, const char *path);
	void end();

	void beginFrame(Cygnet::Color clearColor);
	void endFrame();
	Swan::Vec2i size();

private:
	struct Impl;

	void flush();
	bool openEncoder(int w, int h, int fps, const char *path);

	std::unique_ptr<Impl> impl_;
};

}
