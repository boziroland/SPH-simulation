#ifndef SPHSIM_PARTICLEDATA_H
#define SPHSIM_PARTICLEDATA_H

#include <CL/cl_platform.h>

struct ParticleData{

	struct MyVec2f{
		float x, y;
	};

	cl_float radius;
	cl_float pressure;
	cl_float density;

	MyVec2f velocity;
	MyVec2f pressureForce;
	MyVec2f viscosityForce;

	cl_float mass;
	cl_float effectRadius;
};

#endif //SPHSIM_PARTICLEDATA_H