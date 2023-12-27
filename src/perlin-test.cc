#include <swan/log.h>

#include <PerlinNoise/PerlinNoise.hpp>
#include <png++/png.hpp>

static int getGrassLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise(x / 50.0, 0) * 13);
}

static int getStoneLevel(const siv::PerlinNoise &perlin, int x)
{
	return (int)(perlin.noise(x / 50.0, 10) * 10) + 10;
}

int main()
{
	siv::PerlinNoise perlin = siv::PerlinNoise(100);

	int x1 = -1600;
	int x2 = 1600;
	int y1 = -800;
	int y2 = 800;

	png::image<png::gray_pixel> image(x2 - x1 + 1, y2 - y1 + 1);

	Swan::info << "perlin-test.png: " << (x2 - x1 + 1) << "x" << (y2 - y1 + 1);

	for (int x = x1; x <= x2; ++x) {
		int px = x - x1;
		int grassLevel = getGrassLevel(perlin, x);
		int stoneLevel = getStoneLevel(perlin, x);

		for (int y = y1; y <= y2; ++y) {
			int py = y - y1;

			if (y > grassLevel + 10) {
				double l = perlin.noise(x / 41.37, y / 16.37);
				if (l > 0.2) {
					image[py][px] = 255;
				}
				else{
					image[py][px] = 0;
				}
			}
			else if (y >= grassLevel) {
				image[py][px] = 0;
			}
			else {
				image[py][px] = 255;
			}

			if (y == grassLevel) {
				image[py][px] = 128;
			}
		}
	}

	image.write("perlin-test.png");
}
