#ifndef SPHSIM_PARTICLEDATA_H
#define SPHSIM_PARTICLEDATA_H

#include <CL/cl_platform.h>

struct ParticleData{

	struct MyVec2f{
		cl_float x, y;
	};

	cl_float radius;
	cl_float pressure;
	cl_float density;
	cl_float dummy;

	MyVec2f velocity;
	MyVec2f pressureForce;
	MyVec2f viscosityForce;
	MyVec2f position;

	cl_float mass;
	cl_float effectRadius;
	cl_float dummy2;
	cl_float dummy3;
};

#endif //SPHSIM_PARTICLEDATA_H