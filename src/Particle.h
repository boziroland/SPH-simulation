#ifndef SPHSIM_PARTICLE_H
#define SPHSIM_PARTICLE_H

#include <SFML/Graphics.hpp>
#include "Constants.h"
#include <random>
#include "ParticleData.h"

class Particle {
private:
	sf::CircleShape shape;
	ParticleData data;

public:
	Particle(float, float);
	explicit Particle(Vec2f);

	Vec2f getCenterPos() const;
	void setCenterPos(Vec2f);
	void setCenterPos(float, float);
	sf::CircleShape &getShape();
	const sf::CircleShape &getShape() const;
	void setShape(const sf::CircleShape &shape);
	Vec2f getVelocity() const;
	void setVelocity(Vec2f);
	void setVelocity(float, float);
	float getMass() const;
	float getPressure() const;
	void setPressure(float pressure);
	float getDensity() const;
	void setDensity(float density);
	Vec2f getPressureForce() const;
	void setPressureForce(Vec2f pressureForce);
	Vec2f getViscosityForce() const;
	void setViscosityForce(Vec2f viscosityForce);
	const ParticleData &getData() const;
	void setData(const ParticleData &data);
};

#endif //SPHSIM_PARTICLE_H