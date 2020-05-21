#ifndef SPHSIM_BOUNDARIES_H
#define SPHSIM_BOUNDARIES_H

#include <SFML/Graphics.hpp>

class Boundaries {
private:
	sf::RectangleShape left;
	sf::RectangleShape bottom;
	sf::RectangleShape right;
public:
	Boundaries();
	std::vector<sf::RectangleShape> getShapes();
};

#endif //SPHSIM_BOUNDARIES_H
