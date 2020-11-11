#ifndef SPHSIM_BOUNDARIES_H
#define SPHSIM_BOUNDARIES_H

#include <SFML/Graphics.hpp>

class Boundaries {
private:
	sf::RectangleShape left;
	sf::RectangleShape bottom;
	sf::RectangleShape right;
	sf::ConvexShape triangle;
public:
	Boundaries();
	std::vector<sf::RectangleShape> getShapes();
	const sf::ConvexShape &getTriangle() const;
};

#endif //SPHSIM_BOUNDARIES_H
