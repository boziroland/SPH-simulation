#include "Boundaries.h"

Boundaries::Boundaries() {
	right.setPosition(1260.0f, 0.0f);
	right.setSize(sf::Vector2f{20, 720});
	right.setFillColor(sf::Color::Blue);

	bottom.setPosition(0.0f, 700.0f);
	bottom.setSize(sf::Vector2f{1280, 20});
	bottom.setFillColor(sf::Color::Blue);

	left.setPosition(0.0f, 0.0f);
	left.setSize(sf::Vector2f{20, 720});
	left.setFillColor(sf::Color::Blue);

	triangle.setPointCount(3);
	triangle.setPoint(0, sf::Vector2f{320, 720});
	triangle.setPoint(1, sf::Vector2f{640, 360});
	triangle.setPoint(2, sf::Vector2f{960, 720});
	triangle.setFillColor(sf::Color::Magenta);
}

std::vector<sf::RectangleShape> Boundaries::getShapes() {
	std::vector<sf::RectangleShape> ret;
	ret.push_back(left);
	ret.push_back(bottom);
	ret.push_back(right);

	return ret;
}

const sf::ConvexShape &Boundaries::getTriangle() const {
	return triangle;
}