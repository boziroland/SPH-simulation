#ifndef SPHSIM_TIMER_H
#define SPHSIM_TIMER_H

#include <chrono>
#include <iostream>

class Timer{
	std::chrono::time_point<std::chrono::high_resolution_clock> startPoint;
public:
	inline Timer(){startPoint = std::chrono::high_resolution_clock::now(); }
	inline ~Timer(){
		auto endPoint = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(startPoint).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endPoint).time_since_epoch().count();

		auto duration = end - start;
		float ms = duration * 0.001f;

		std::cout << duration << " us (" << ms << " ms)\n";
	}
};

#endif //SPHSIM_TIMER_H
