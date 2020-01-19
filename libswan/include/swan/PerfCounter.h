#include "util.h"

#include <array>

namespace Swan {

class PerfCounter {
public:
	template<typename T = double, size_t size = 64>
	class Counter {
	public:
		void count(T val) {
			buf_[idx_] = val;
			idx_ = (idx_ + 1) % size;
		}

		// Fill a buffer with data, in the order they were written
		template<typename DestT = T>
		void fill(std::array<DestT, size> &buf) {
			size_t r = idx_;
			size_t w = 0;
			do {
				buf[w++] = (DestT)buf_[r];
				r = (r + 1) % size;
			} while (r != idx_);
		}

	private:
		T buf_[size] = {};
		size_t idx_ = 0;
	};

	void render();

	void countTotalTime(double secs) { total_time_.count(secs); }
	void countFrameTime(double secs) { frame_time_.count(secs); }
	void countGameUpdate(double secs) { game_update_.count(secs); }
	void countGameTick(double secs) { game_tick_.count(secs); }
	void countGameDraw(double secs) { game_draw_.count(secs); }
	void countGameUpdatesPerFrame(double count) { game_updates_per_frame_.count(count); }
	void countRenderPresent(double secs) { render_present_.count(secs); }
	void countMemoryUsage(double bytes) { memory_usage_.count(bytes); }

private:
	Counter<> total_time_;
	Counter<> frame_time_;
	Counter<> game_update_;
	Counter<> game_tick_;
	Counter<> game_draw_;
	Counter<> game_updates_per_frame_;
	Counter<> render_present_;
	Counter<> memory_usage_;
};

}
