#include "drawutil.h"

#include <algorithm>

namespace Swan {

namespace Draw {

static float linearLine(float from, float to, float frac)
{
	return std::clamp(to * frac + from * (1 - frac), 0.0f, 255.0f);
}

static Cygnet::Color linearColor(Cygnet::Color from, Cygnet::Color to, float frac)
{
	return {
		.r = linearLine(from.r, to.r, frac),
		.g = linearLine(from.g, to.g, frac),
		.b = linearLine(from.b, to.b, frac),
		.a = linearLine(from.a, to.a, frac),
	};
}

Cygnet::Color linearGradient(
	float val,
	std::initializer_list<std::pair<float, Cygnet::Color> > colors)
{
	const std::pair<float, Cygnet::Color> *arr = colors.begin();
	size_t size = colors.size();

	if (val < arr[0].first) {
		return arr[0].second;
	}

	for (size_t i = 1; i < size; ++i) {
		if (arr[i].first < val) {
			continue;
		}

		auto [fromv, fromc] = arr[i - 1];
		auto [tov, toc] = arr[i];
		float frac = (val - fromv) / (tov - fromv);
		return linearColor(fromc, toc, frac);
	}

	return arr[size - 1].second;
}

}

}
