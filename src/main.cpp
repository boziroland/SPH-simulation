#include <SFML/Graphics.hpp>
#include "Particle.h"
#include "Boundaries.h"
#include <chrono>
#include <random>
#include <iostream>
#include "opencl/OpenCLHelper.h"
#include "Timer.h"

const std::string fileToSaveTo = "../data/safety.csv";

#define WIDTH 1280.0f
#define HEIGHT 720.0f

const bool triangleCollision = false;

std::vector<Particle> initParticles();

void addParticles(std::vector<Particle> &particles);

void update(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders,
			OpenCLHelper &openClHelper);

void
draw(sf::RenderWindow &window, const std::vector<Particle> &particles, const std::vector<sf::RectangleShape> &borders);

void passDataToGPU(std::vector<Particle> &particles, const std::string &gpuFunction, OpenCLHelper &helper);

void move(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders);

void checkBounds(Particle &p);

float length(const Vec2f &vec) {
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
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
	int frames = 0;
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
						//std::cout << "FRAMES: " << frames << std::endl;
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
		frames++;
		if(frames >= 1000)
			break;
	}

	std::ofstream file(fileToSaveTo);
	for(const auto& e : Timer::frameFrameTimeMap)
		file << e.first << ";" << e.second << "\n";

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

void update(float dt, std::vector<Particle> &particles, std::vector<sf::RectangleShape> &borders, OpenCLHelper &openClHelper) {
	Timer timer;
	passDataToGPU(particles, "THISPARAMISDEPRECATED", openClHelper);
}

void
draw(sf::RenderWindow &window, const std::vector<Particle> &particles, const std::vector<sf::RectangleShape> &borders) {
	window.clear();
	for (const auto &p : particles) {
		window.draw(p.getShape());
	}

	if(triangleCollision) {
		window.draw(boundaries.getTriangle());
	}

	for (const auto &b : borders) {
		window.draw(b);
	}

	window.display();
}