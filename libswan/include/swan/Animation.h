#pragma once

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace Swan {

class Animation {
public:
	Animation(int w, int h, double freq, const sf::Image &img);
	Animation(int w, int h, double freq, const std::string &path);

	void tick(double dt);
	bool fill(sf::Texture &tex, bool force = false);

	int width_, height_;

private:
	sf::Image img_;
	int fcount_;
	int frame_ = 0;
	bool dirty_ = true;
	double interval_;
	double time_ = 0;
};

}
