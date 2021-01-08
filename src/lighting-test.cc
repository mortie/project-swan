#include <swan/LightServer.h>
#include <swan/log.h>
#include <png++/png.hpp>
#include <chrono>

class CB final: public Swan::LightCallback {
public:
	void onLightChunkUpdated(const Swan::LightChunk &chunk, Swan::Vec2i pos) final {
		Swan::info << "light chunk at " << pos;
		chunk_ = chunk;
		done_ = true;
		cond_.notify_one();
	}

	Swan::LightChunk chunk_;
	bool done_ = false;
	std::mutex mut_;
	std::condition_variable cond_;
};

int main() {
	CB cb;
	Swan::LightServer lt(cb);

	Swan::NewLightChunk nc;
	auto set = [&](int x, int y) { nc.blocks[y * Swan::CHUNK_WIDTH + x] = true; };
	set(0, 0);
	set(18, 3);
	set(12, 13);
	set(28, 22);
	set(22, 12);
	for (int x = 4; x < 28; ++x) {
		set(x, 24);
	}
	for (int x = 12; x < 20; ++x) {
		set(x, 26);
	}
	nc.lightSources = {
		{ { 20, 10 }, 20 },
		{ { 16, 30 }, 20 },
		{ { 5, 27 }, 20 },
	};

	lt.onChunkAdded({0, 0}, std::move(nc));

	std::unique_lock<std::mutex> lock(cb.mut_);
	cb.cond_.wait(lock, [&] { return cb.done_; });
	cb.done_ = false;

	lt.onSolidBlockAdded({ 10, 10 });
	cb.cond_.wait(lock, [&] { return cb.done_; });
	cb.done_ = false;

	png::image<png::rgb_pixel> image(Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT);
	for (int y = 0; y < Swan::CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < Swan::CHUNK_WIDTH; ++x) {
			uint8_t light = cb.chunk_.lightLevels[y * Swan::CHUNK_WIDTH + x];
			bool block = false;
			if (cb.chunk_.blocks[y * Swan::CHUNK_WIDTH + x]) {
				block = true;
			}

			bool isLight =
				(x == 20 && y == 10) ||
				(x == 16 && y == 30) ||
				(x == 5  && y == 27);

			unsigned char lightcol = (unsigned char)(sqrt(light) * 30);
			if (block) {
				image[y][x] = {
					lightcol, lightcol, lightcol };
			} else if (isLight) {
				image[y][x] = {
					255, 255, 64 };
			} else {
				image[y][x] = {
					lightcol, 0, 0 };
			}
		}
	}

	image.write("lighting-test.png");
}
