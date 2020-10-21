#include "Particle.h"

Particle::Particle(float x, float y) {
	data = ParticleData{};
	data.radius = particle_radius;
	data.effectRadius = effect_radius;
	data.mass = mass_const;

	shape.setPosition(x, y);
	shape.setRadius(data.radius);
	shape.setFillColor(sf::Color::White);

	data.pressure = 0.0f;
	data.velocity.x = 0.0f;
	data.velocity.y = 0.0f;
	data.density = 0.0f;
}

Particle::Particle(Vec2f pos) {
	data = ParticleData{};
	data.radius = particle_radius;

	shape.setPosition(pos);
	shape.setRadius(data.radius);

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<unsigned int> dist(0, 255);

	shape.setFillColor(sf::Color{static_cast<sf::Uint8>(dist(mt)), static_cast<sf::Uint8>(dist(mt)),
								 static_cast<sf::Uint8>(dist(mt))});

	data.pressure = 0.0f;
	data.velocity.x = 0.0f;
	data.velocity.y = 0.0f;
	data.density = 0.0f;
}

sf::CircleShape &Particle::getShape() {
	return shape;
}

const sf::CircleShape &Particle::getShape() const {
	return shape;
}

Vec2f Particle::getCenterPos() const {
	auto pos = shape.getPosition();
	pos.x += data.radius;
	pos.y += data.radius;
	return pos;
}

void Particle::setCenterPos(Vec2f pos) {
	pos.x -= data.radius;
	pos.y -= data.radius;
	shape.setPosition(pos.x, pos.y);
}

Vec2f Particle::getVelocity() const {
	return Vec2f{data.velocity.x, data.velocity.y};
}

void Particle::setVelocity(Vec2f _velocity) {
	data.velocity.x = _velocity.x;
	data.velocity.y = _velocity.y;
}

float Particle::getMass() const {
	return data.mass;
}

float Particle::getPressure() const {
	return data.pressure;
}

void Particle::setPressure(float pressure) {
	data.pressure = pressure;
}

float Particle::getDensity() const {
	return data.density;
}

void Particle::setDensity(float density) {
	data.density = density;
}

Vec2f Particle::getPressureForce() const {
	return Vec2f{data.pressureForce.x, data.pressureForce.y};
}

void Particle::setPressureForce(Vec2f pressureForce) {
	data.pressureForce.x = pressureForce.x;
	data.pressureForce.y = pressureForce.y;
}

Vec2f Particle::getViscosityForce() const {
	return Vec2f{data.viscosityForce.x, data.viscosityForce.y};
}

void Particle::setViscosityForce(Vec2f viscosityForce) {
	data.viscosityForce.x = viscosityForce.x;
	data.viscosityForce.y = viscosityForce.y;
}

void Particle::setVelocity(float x, float y) {
	data.velocity.x = x;
	data.velocity.y = y;
}

void Particle::setCenterPos(float x, float y) {
	x -= data.radius;
	y -= data.radius;

	shape.setPosition(x, y);
}

void Particle::setShape(const sf::CircleShape &shape) {
	Particle::shape = shape;
}

const ParticleData &Particle::getData() const {
	return data;
}

void Particle::setData(const ParticleData &data) {
	Particle::data = data;
}
