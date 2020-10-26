typedef struct MyVec2f{
    float x;
    float y;
} MyVec2f;

typedef struct ParticleData{
	int radius;
    float pressure;
    float density;
    float dummy;

    MyVec2f velocity;
    MyVec2f pressureForce;
    MyVec2f viscosityForce;
    MyVec2f position;

    float mass;
    float effectRadius;
    float dummy2;
    float dummy3;
} ParticleData;

__constant float PI = 3.14159265359f;
__constant float background_pressure = 1750.0f;
__constant float default_fluid_density = 1250.0f;

float poly6_kernel(float2 vec, float h){
    float dist_sq = vec.x * vec.x + vec.y * vec.y;
    float h_sq = h * h;
    if(h_sq < dist_sq || dist_sq < 0.0f){
        return 0.0f;
    }else{
        return 315.0f / (64.0f * PI * pow(h, 9.0f)) * pow(h_sq - dist_sq, 3.0f);
    }
}

__kernel void calculateDensity(__global ParticleData* inputData, __global int* size){
    int id = get_global_id(0);
    ParticleData pi = inputData[id];
    float dens = 0.0f;
    for(int i = 0; i < (*size); i++){

            ParticleData pj = inputData[i];
            float2 distanceVector = (float2)(pi.position.x - pj.position.x, pi.position.y - pj.position.y);
            float distance = fast_length(distanceVector);

            if(distance < pi.effectRadius){
                float mass = pj.mass;
                float kernel_val = poly6_kernel(distanceVector, pi.effectRadius);
                float currDens = mass * kernel_val;
                dens += currDens;
            }
    }
    float currPress = background_pressure * (dens - default_fluid_density);

    inputData[id].density = dens;
    inputData[id].pressure = currPress;
}

__kernel void square1(__global ParticleData* inputData, __global ParticleData* outputData){
  int id = get_global_id(0);
  outputData[id].radius = inputData[id].radius * inputData[id].radius;
}