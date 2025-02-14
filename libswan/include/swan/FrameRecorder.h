#pragma once

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

private:
	struct Impl;

	void flush();

	std::unique_ptr<Impl> impl_;
};

}
