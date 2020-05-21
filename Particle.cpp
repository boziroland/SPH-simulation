#include "Particle.h"

float Particle::effectRadius = effect_radius;
float Particle::mass = mass_const;

Particle::Particle(float x, float y) {
	radius = particle_radius;

	shape.setPosition(x, y);
	shape.setRadius(radius);
	shape.setFillColor(sf::Color::White);

	pressure = 0.0f;
	velocity = Vec2f{0.0f, 0.0f};
	density = 0.0f;
}


Particle::Particle(Vec2f pos){
	radius = particle_radius;

	shape.setPosition(pos);
	shape.setRadius(radius);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<unsigned int> dist(0, 255);

	shape.setFillColor(sf::Color{static_cast<sf::Uint8>(dist(mt)), static_cast<sf::Uint8>(dist(mt)), static_cast<sf::Uint8>(dist(mt))});

	pressure = 0.0f;
	velocity = Vec2f{0.0f, 0.0f};
	density = 0.0f;
}


sf::CircleShape &Particle::getShape() {
	return shape;
}

const sf::CircleShape &Particle::getShape() const{
	return shape;
}

Vec2f Particle::getCenterPos() const {
	auto pos = shape.getPosition();
	pos.x += radius;
	pos.y += radius;
	return pos;
}

void Particle::setCenterPos(Vec2f pos){
	pos.x -= radius;
	pos.y -= radius;
	shape.setPosition(pos.x, pos.y);
}

Vec2f Particle::getVelocity() const {
	return velocity;
}

void Particle::setVelocity(Vec2f _velocity) {
	Particle::velocity = _velocity;
}

float Particle::getMass() const {
	return mass;
}

float Particle::getPressure() const {
	return pressure;
}

void Particle::setPressure(float pressure) {
	Particle::pressure = pressure;
}

float Particle::getDensity() const {
	return density;
}

void Particle::setDensity(float density) {
	Particle::density = density;
}

Vec2f Particle::getPressureForce() const {
	return pressureForce;
}

void Particle::setPressureForce(Vec2f pressureForce) {
	Particle::pressureForce = pressureForce;
}

Vec2f Particle::getViscosityForce() const {
	return viscosityForce;
}

void Particle::setViscosityForce(Vec2f viscosityForce) {
	Particle::viscosityForce = viscosityForce;
}

void Particle::setVelocity(float x, float y) {
	velocity.x = x;
	velocity.y = y;
}

void Particle::setCenterPos(float x, float y) {
	x -= radius;
	y -= radius;

	shape.setPosition(x, y);
}