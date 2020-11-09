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

#define WIDTH 1280.0f
#define HEIGHT 720.0f

__constant float PI = 3.14159265359f;
__constant float background_pressure = 1750.0f;
__constant float default_fluid_density = 1250.0f;
__constant float viscosity = 600.0f;
__constant float2 gravity = (float2)(0.0f, 15000.0f * 9.81f);

__constant float wallThicknessLeft = 36.0f;
__constant float wallThicknessRight = 45.0f;
__constant float floorHeight = 36.0f;

__constant float dt = 0.0005f;

float poly6_kernel(float2 vec, float h){
    float dist_sq = vec.x * vec.x + vec.y * vec.y;
    float h_sq = h * h;
    if(h_sq < dist_sq || dist_sq < 0.0f){
        return 0.0f;
    }else{
        return 315.0f / (64.0f * PI * pow(h, 9.0f)) * pow(h_sq - dist_sq, 3.0f);
    }
}

float gradKernel_pressure(float2 vec, float h){
    float r = sqrt(vec.x * vec.x + vec.y * vec.y);
    if(r == 0.0f)
        return 0.0f;
    else
        return 45.0f / (PI * pow(h, 6.0f)) * pow(h - r, 2.0f);
}

float laplaceKernel_viscosity(float2 vec, float h){
    float r = sqrt(vec.x * vec.x + vec.y * vec.y);
    return 45.0f / (PI * pow(h, 6.0f)) * (h - r);
}

void checkBounds(__global ParticleData* p){
    float2 velocity = (float2)(p->velocity.x, p->velocity.y);
    float2 pos = (float2)(p->position.x, p->position.y);

    if(pos.x < wallThicknessLeft){
        velocity.x = p->velocity.x = velocity.x * (-0.5f);
        pos.x = p->position.x = wallThicknessLeft;

    }else if(pos .x + wallThicknessRight > WIDTH){
        velocity.x = p->velocity.x = velocity.x * (-0.5f);
        pos.x = p->position.x = WIDTH - wallThicknessRight;

    }

    if(pos.y + floorHeight > HEIGHT){
        p->velocity.y = velocity.y * (-0.5f);
        p->position.y = HEIGHT - floorHeight;
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

__kernel void calculatePressureAndViscosityForce(__global ParticleData* inputData, __global int* size){
    int id = get_global_id(0);
    ParticleData pi = inputData[id];

    float2 forceP = (float2)(0.0f, 0.0f);
    float2 forceV = (float2)(0.0f, 0.0f);
    for(int i = 0; i < (*size); i++){
        ParticleData pj = inputData[i];
            if(&inputData[id] != &inputData[i]){
                float2 distanceVector = (float2)(pi.position.x - pj.position.x, pi.position.y - pj.position.y);
                float distance = fast_length(distanceVector);

                if(distance < pi.effectRadius){
                    float2 effect_normalized = (-1.0f) * normalize(distanceVector);
                    float kernelP = gradKernel_pressure(distanceVector, pi.effectRadius);
                    float kernelV = laplaceKernel_viscosity(distanceVector, pi.effectRadius);

                    float2 currPForce = effect_normalized * pj.mass * (pi.pressure + pj.pressure) / (pj.density * 2.0f) * kernelP;
                    float2 piVelocity = (float2)(pi.velocity.x, pi.velocity.y);
                    float2 pjVelocity = (float2)(pj.velocity.x, pj.velocity.y);
                    float2 currVisc = viscosity * pi.mass * (pjVelocity - piVelocity) / pj.density * kernelV;

                    forceP += currPForce;
                    forceV += currVisc;
                }
        }
    }

    inputData[id].viscosityForce.x = forceV.x;
    inputData[id].viscosityForce.y = forceV.y;
    inputData[id].pressureForce.x = forceP.x;
    inputData[id].pressureForce.y = forceP.y;
}

__kernel void move(__global ParticleData* inputData, __global int* size){
    int id = get_global_id(0);
    float2 currVel = (float2)(inputData[id].velocity.x, inputData[id].velocity.y);
    float2 currVisF = (float2)(inputData[id].viscosityForce.x, inputData[id].viscosityForce.y);
    float2 currPreF = (float2)(inputData[id].pressureForce.x, inputData[id].pressureForce.y);
    float2 currPos = (float2)(inputData[id].position.x, inputData[id].position.y);
    float currDens = inputData[id].density;

    float2 newVel = currVel + (dt * (currVisF + currPreF + gravity * currDens) / currDens);
    float2 newPos = currPos + dt * newVel;

    inputData[id].velocity.x = newVel.x;
    inputData[id].velocity.y = newVel.y;

    inputData[id].position.x = newPos.x;
    inputData[id].position.y = newPos.y;

    checkBounds(&inputData[id]);
}

__kernel void updateForces(__global ParticleData* inputData, __global int* size){
    calculateDensity(inputData, size);
    barrier(CLK_GLOBAL_MEM_FENCE);
    calculatePressureAndViscosityForce(inputData, size);
}

__kernel void updatePosition(__global ParticleData* inputData, __global int* size){
    move(inputData, size); //ezt másik kernelhívásba
}

//egy rámpát ha bele kéne tenni az jó legyen