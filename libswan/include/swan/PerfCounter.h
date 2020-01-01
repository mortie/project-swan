namespace Swan {



class PerfCounter {
public:
	class Counter {
	public:
		void count(double val) {
			latest_ = val;
			if (count_ >= 60) {
				sum_ -= avg();
				sum_ += val;
			} else {
				sum_ += val;
				count_ += 1;
			}
		}

		double avg() { return sum_ / count_; }
		double latest() { return latest_; }

	private:
		double latest_ = 0;
		double sum_ = 0;
		int count_ = 0;
	};

	void countFrameTime(double secs) { frame_time_.count(secs); }
	void countGameUpdate(double secs) { game_update_.count(secs); }
	void countGameTick(double secs) { game_tick_.count(secs); }
	void countGameDraw(double secs) { game_draw_.count(secs); }
	void countGameUpdatesPerFrame(double count) { game_updates_per_frame_.count(count); }
	void countRenderPresent(double secs) { render_present_.count(secs); }
	void countMemoryUsage(double bytes) { memory_usage_.count(bytes); }

private:
	Counter frame_time_;
	Counter game_update_;
	Counter game_tick_;
	Counter game_draw_;
	Counter game_updates_per_frame_;
	Counter render_present_;
	Counter memory_usage_;
};

}
