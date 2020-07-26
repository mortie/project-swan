#pragma once

#include <memory>
#include <stdlib.h>

#include "GlWrappers.h"

namespace Cygnet {

class Image {
public:
	Image(std::string path);

	GlTexture &texture();

private:
	std::string path_;

	int w_, h_, pitch_;
	CPtr<void, free> bytes_;

	GlTexture tex_;
	bool tex_dirty_ = true;
};

}
