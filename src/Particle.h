#ifndef SPHSIM_PARTICLE_H
#define SPHSIM_PARTICLE_H

#include <SFML/Graphics.hpp>
#include "Constants.h"
#include <random>

class Particle {
private:

	sf::CircleShape shape;
//	float radius;
//
//	float pressure;
//	Vec2f velocity;
//	float density;
//
//	Vec2f pressureForce;
//	Vec2f viscosityForce;

public:
//	static float mass;
//	static float effectRadius;

	struct Data{

		struct MyVec2f{
			float x, y;
		};

		float radius;
		float pressure;
		MyVec2f velocity;
		float density;

		MyVec2f pressureForce;
		MyVec2f viscosityForce;

		static float mass;
		static float effectRadius;
	};

	Data data{};

public:
	Particle(float, float);
	Particle(float, float, bool);
	explicit Particle(Vec2f);

	const Data &getData() const;

	void setData(const Data &data);

	Vec2f getCenterPos() const;
	void setCenterPos(Vec2f);
	void setCenterPos(float, float);
	sf::CircleShape &getShape();
	const sf::CircleShape &getShape() const;
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
};

#endif //SPHSIM_PARTICLE_H