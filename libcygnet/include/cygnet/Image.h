#pragma once

#include <memory>

#include "GlWrappers.h"

namespace Cygnet {

class Image {
public:
	Image(std::string path);

	GlTexture &texture();
	int width() { return w_; }
	int height() { return h_; }

private:
	std::string path_;

	int w_, h_, pitch_;
	std::unique_ptr<unsigned char[]> bytes_;

	GlTexture tex_;
	bool tex_dirty_ = true;
};

}
