typedef struct MyVec2f{
    float x;
    float y;
} MyVec2f;

typedef struct ParticleData{
	int radius;
    float pressure;
    float density;

    MyVec2f velocity;
    MyVec2f pressureForce;
    MyVec2f viscosityForce;

    float mass;
    float effectRadius;
} ParticleData;

__kernel void square(__global int* inputData, __global int* outputData){
  int id = get_global_id(0);
  outputData[id] = inputData[id] * inputData[id];
}
__kernel void square1(__global ParticleData* inputData, __global ParticleData* outputData){
  int id = get_global_id(0);
  outputData[id].radius = inputData[id].radius * inputData[id].radius;
}