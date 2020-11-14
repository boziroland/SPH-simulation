#include <SFML/Graphics.hpp>
#include "Particle.h"
#include "Boundaries.h"
#include <chrono>
#include <random>
#include <iostream>
#include "opencl/OpenCLHelper.h"
#include "opencl/Common.h"

//#define CL_HPP_ENABLE_EXCEPTIONS

#define WIDTH 1280.0f
#define HEIGHT 720.0f

const bool triangleCollision = false;

std::vector<Particle> initParticles();

void addParticles(std::vector<Particle> &particles);

void update(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders,
			OpenCLHelper &openClHelper);

void
draw(sf::RenderWindow &window, const std::vector<Particle> &particles, const std::vector<sf::RectangleShape> &borders);

void calculateDensity(std::vector<Particle> &particles);

void passDataToGPU(std::vector<Particle> &particles, const std::string &gpuFunction, OpenCLHelper &helper);

void calculatePressureAndViscosityForce(std::vector<Particle> &particles);

void move(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders);

void checkBounds(Particle &p);

float length(const Vec2f &vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

Vec2f normalize(const Vec2f &vec) {
	float len = length(vec);
	return Vec2f{vec.x / len, vec.y / len};
}

float gaussian_kernel(Vec2f _r, float h) {
	float r = std::sqrt(_r.x * _r.x + _r.y * _r.y);
	float alpha = 1 / ((std::sqrt(PI) * h) * (std::sqrt(PI) * h));
	return alpha * std::exp(-((r / h) * (r / h)));
}

float poly6_kernel(Vec2f _r, float h) {
	float dist_sq = _r.x * _r.x + _r.y * _r.y;
	float h_sq = h * h;
	return h_sq < dist_sq || dist_sq < 0 ? 0.0f : 315.0f / (64.0f * PI * std::pow(h, 9.0f)) *
												  std::pow(h_sq - dist_sq, 3.0f);
}

float kernel(Vec2f _r, float h) {
	return poly6_kernel(_r, h);
}

float gradKernel_pressure(Vec2f x, float h) {
	float r = std::sqrt(x.x * x.x + x.y * x.y);
	return r == 0.0f ? 0.0f : 45.0f / (PI * std::pow(h, 6.0f)) * std::pow(h - r, 2.0f);
}

float laplaceKernel_viscosity(Vec2f x, float h) {
	float r = std::sqrt(x.x * x.x + x.y * x.y);
	return 45.0f / (PI * std::pow(h, 6.0f)) * (h - r);
}

Boundaries boundaries;

int main() {

	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;
	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Fluid Simulation", sf::Style::Default, settings);
	window.setFramerateLimit(60);

	std::vector<Particle> particles = initParticles();
	auto borders = boundaries.getShapes();

	OpenCLHelper openClHelper{R"(F:\Egyetem\onlab\SPHsim\src\opencl\programs.cl)", particles};

	while (window.isOpen()) {

		float dtime = 0.0005f;

		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {

				case sf::Event::Closed:
					window.close();
					break;

				case sf::Event::KeyReleased:

					if (event.key.code == sf::Keyboard::Enter) { //if enter is released, add more particles
						addParticles(particles);
					} else if (event.key.code == sf::Keyboard::Space) { //if space is released, restart
						particles.clear();
						particles = initParticles();
					}
					break;

				default:
					break;
			}
		}

		update(dtime, particles, borders, openClHelper);
		draw(window, particles, borders);
	}

	return 0;
}

void passDataToGPU(std::vector<Particle> &particles, const std::string &gpuFunction, OpenCLHelper &helper) {
	int err = CL_SUCCESS;
	int bufferSize = particles.size();

	auto program = helper.getProgram();
	auto context = helper.getContext();
	auto devices = helper.getDevices();
	auto kernelUpdate = helper.updateKernel;
	auto kernelMove = helper.moveKernel;
	auto clInputBuffer = helper.inputBuffer;
	auto clCountBuffer = helper.countBuffer;

	try {

		std::vector<ParticleData> hostBuffer;
		for (size_t i = 0; i < bufferSize; i++) {
			hostBuffer.push_back(particles[i].getData());
		}

		cl::Event event;

		cl::CommandQueue myQueue(context, devices[0], 0, &err);
		err = myQueue.enqueueWriteBuffer(clInputBuffer, true, 0, sizeof(ParticleData) * bufferSize, hostBuffer.data());
		//std::cout << getErrorString(err) << std::endl;
		err = myQueue.enqueueWriteBuffer(clCountBuffer, true, 0, sizeof(int), &bufferSize);
		//std::cout << getErrorString(err) << std::endl;

		err = myQueue.enqueueNDRangeKernel(kernelUpdate,
										   cl::NullRange,
										   cl::NDRange(bufferSize, 1),
										   cl::NullRange, //egy workgroupba hány szál
										   NULL,
										   &event);

		err = myQueue.enqueueNDRangeKernel(kernelMove,
										   cl::NullRange,
										   cl::NDRange(bufferSize, 1),
										   cl::NullRange, //egy workgroupba hány szál
										   NULL,
										   &event);
		//std::cout << getErrorString(err) << std::endl;
		event.wait();

		std::vector<ParticleData> resultBuffer(bufferSize);
		err = myQueue.enqueueReadBuffer(clInputBuffer, true, 0, sizeof(ParticleData) * bufferSize, resultBuffer.data());
		//std::cout << getErrorString(err) << std::endl;

		for (size_t i = 0; i < bufferSize; i++) {
			particles[i].setData(resultBuffer[i]);
			particles[i].setCenterPos(particles[i].getData().position.x, particles[i].getData().position.y);
		}
	}
	catch (cl::BuildError &e) {
		std::cout << "build error" << std::endl;
		std::cout << e.getBuildLog().front().second << std::endl;
	}
	catch (cl::Error &e) {
		std::cout << e.what() << std::endl;
	}
}

void calculateDensity(std::vector<Particle> &particles) {

	for (auto &pi : particles) {
		float dens = 0.0f;
		for (auto &pj : particles) {

			Vec2f distanceVector = pi.getCenterPos() - pj.getCenterPos();
			float distance = length(distanceVector);

			if (distance < pi.getData().effectRadius) {
				auto mass = pj.getMass();
				auto kernel_val = kernel(distanceVector, pi.getData().effectRadius);
				float currDens = mass * kernel_val;
				dens += currDens;
			}
		}
		auto currPress = background_pressure * (dens - default_fluid_density);

		pi.setDensity(dens);
		pi.setPressure(currPress);
	}
}

void calculatePressureAndViscosityForce(std::vector<Particle> &particles) {
	for (auto &pi : particles) {
		Vec2f forceP;
		Vec2f forceV;

		for (auto &pj : particles) {
			if (&pi != &pj) {

				Vec2f distanceVector = pi.getCenterPos() - pj.getCenterPos();
				float distance = length(distanceVector);

				if (distance < pi.getData().effectRadius) {

					Vec2f effect_normalized = -normalize(distanceVector);
					auto kernelP = gradKernel_pressure(distanceVector, pi.getData().effectRadius);

					Vec2f currPForce = effect_normalized * pj.getMass() *
									   (pi.getPressure() + pj.getPressure()) /
									   (pj.getDensity() * 2.0f) *
									   kernelP;

					Vec2f currVisc = viscosity * pi.getMass() * (pj.getVelocity() - pi.getVelocity()) /
									 pj.getDensity() *
									 laplaceKernel_viscosity(distanceVector, pi.getData().effectRadius);

					forceP += currPForce;
					forceV += currVisc;
				}
			}
		}

		if (!std::isnan(forceP.x)) {
			pi.setViscosityForce(forceV);
			pi.setPressureForce(forceP);
		}
	}
}

void move(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders) {

	for (auto &p : particles) {
		auto currVel = p.getVelocity();
		auto currVisF = p.getViscosityForce();
		auto currPreF = p.getPressureForce();
		auto currDens = p.getDensity();

		p.setVelocity(currVel + (dt * (currVisF + currPreF + gravity * currDens) / currDens));
		p.setCenterPos(p.getCenterPos() + dt * p.getVelocity());

		checkBounds(p);
	}
}

void checkBounds(Particle &p) {
	auto velocity = p.getVelocity();
	auto pos = p.getCenterPos();

	if (pos.x < wallThicknessLeft) {

		p.setVelocity(velocity.x * (-0.5f), velocity.y);
		p.setCenterPos(36.0f, pos.y);

		velocity = p.getVelocity();
		pos = p.getCenterPos();

	} else if (pos.x + wallThicknessRight > WIDTH) {

		p.setVelocity(velocity.x * (-0.5f), velocity.y);
		p.setCenterPos(WIDTH - 45.0f, pos.y);

		velocity = p.getVelocity();
		pos = p.getCenterPos();
	}

//	if (pos.y + floorHeight > HEIGHT) {
//		p.setVelocity(Vec2f{velocity.x, velocity.y * -0.5f});
//		p.setCenterPos(Vec2f{pos.x, HEIGHT - 36.0f});
//	}
}

std::vector<Particle> initParticles() {
	std::vector<Particle> ret;

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	for (int i = 0; i < particle_cols_amount; i++) {
		for (int j = 0; j < particle_rows_amount; j++) {
			auto rand = dist(mt);
			ret.emplace_back(45.0f * i + 430.0f + rand, 200.0f - (45.0f * j));
		}
	}

	return ret;
}

void addParticles(std::vector<Particle> &particles) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			particles.emplace_back(Vec2f{500.0f + i * 30.0f, 100.0f + j * 30.0f});
		}
	}
}

void update(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders,
			OpenCLHelper &openClHelper) {
	passDataToGPU(particles, "update", openClHelper);
}

void
draw(sf::RenderWindow &window, const std::vector<Particle> &particles, const std::vector<sf::RectangleShape> &borders) {
	window.clear();
	for (const auto &p : particles) {
		window.draw(p.getShape());
	}

	if(triangleCollision)
		window.draw(boundaries.getTriangle());

	for (const auto &b : borders) {
		window.draw(b);
	}

	window.display();
}