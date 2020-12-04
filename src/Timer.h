#ifndef SPHSIM_TIMER_H
#define SPHSIM_TIMER_H

#include <chrono>
#include <iostream>
#include <fstream>

using namespace std::chrono;

class Timer{
	inline static int frame = 0;
	time_point<high_resolution_clock> start;
public:
	inline static std::map<int, float> frameFrameTimeMap;
	Timer() : start(high_resolution_clock::now()) {}
	~Timer(){
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(end - start).count();
		frameFrameTimeMap.insert({frame++, duration});
	}
};

#endif //SPHSIM_TIMER_H
