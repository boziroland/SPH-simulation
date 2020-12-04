typedef struct MyVec2f{
    float x;
    float y;
} MyVec2f;

typedef struct ParticleData{
	float radius;
    float pressure;
    float density;

    MyVec2f velocity;
    MyVec2f pressureForce;
    MyVec2f viscosityForce;
    MyVec2f position;

    float mass;
    float effectRadius;
} ParticleData;

#define WIDTH 1280.0f
#define HEIGHT 720.0f

__constant bool triangleCollision = false;

__constant float PI = 3.14159265359f;
__constant float background_pressure = 1750.0f / 2.0f;
__constant float default_fluid_density = 1250.0f / 2.0f;
__constant float viscosity = 600.0f / 2.0f;
__constant float2 gravity = (float2)(0.0f, 30000.0f * 9.81f / 30000.0f);
__constant float non_horizontal_bounce_constant = 0.95f;

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

bool intersects(float circleRadius, float2 circleCenter, float2 point1, float2 point2){

    MyVec2f a = { .x = point1.x - circleCenter.x, .y = point1.y - circleCenter.y};
    MyVec2f b = { .x = point2.x - circleCenter.x, .y = point2.y - circleCenter.y};

    float a0 = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
    float b0 = 2 * (a.x * (b.x - a.x) + a.y * (b.y - a.y));
    float c0 = a.x * a.x + a.y * a.y - (circleRadius + 0.0f) * (circleRadius + 0.0f);
    float discriminant = b0 * b0 - 4 * a0 * c0;

    if(discriminant <= 0)
        return false;

    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-b0 + sqrtDiscriminant) / (2 * a0);
    float t2 = (-b0 - sqrtDiscriminant) / (2 * a0);

    return (0 < t1 && t1 < 1) || (0 < t2 && t2 < 1);
}

float2 getNormal(float2 point1, float2 point2){
    float2 vector = point2 - point1;
    return normalize((float2)(-vector.y, vector.x));
}

void checkParticleTriangleIntersection(__global ParticleData* p){
    float circleRadius = p->radius;
    float2 circleCenter = (float2)(p->position.x, p->position.y);
    float2 p0 = (float2)(320, 720);
    float2 p1 = (float2)(640, 360);
    float2 p2 = (float2)(960, 720);

    float2 velocity = (float2)(p->velocity.x, p->velocity.y);
    float2 pos = (float2)(p->position.x, p->position.y);
    float bounceOffset = 60.0f;
    float2 normal, newVelocity;
    if(triangleCollision){

        if(intersects(circleRadius, circleCenter, p0, p1)){
            normal = getNormal(p0, p1);
            newVelocity = velocity - 2 * (dot(velocity, normal)) * normal;
            p->velocity.x = newVelocity.x * non_horizontal_bounce_constant;
            p->velocity.y = newVelocity.y * non_horizontal_bounce_constant;
            p->position.x = pos.x - circleRadius / bounceOffset;
            p->position.y = pos.y - circleRadius / bounceOffset;
            pos.x = p->position.x;
            pos.y = p->position.y;
        }

        if(intersects(circleRadius, circleCenter, p1, p2)){
            normal = getNormal(p1, p2);
            newVelocity = velocity - 2 * (dot(velocity, normal)) * normal;
            p->velocity.x = newVelocity.x * non_horizontal_bounce_constant;
            p->velocity.y = newVelocity.y * non_horizontal_bounce_constant;
            p->position.x = pos.x + circleRadius / bounceOffset;
            p->position.y = pos.y - circleRadius / bounceOffset;
        }else if(pos.y + 36.0f >= HEIGHT){
            normal = getNormal((float2)(0.0f, 0.0f), (float2)(1.0f, 0.0f));
            newVelocity = velocity - 2 * (dot(velocity, normal)) * normal;
            p->velocity.x = newVelocity.x;
            p->velocity.y = newVelocity.y * 0.5f;
            p->position.x = pos.x;
            p->position.y = HEIGHT - 36.0f;
        }

    }else{
        if(pos.y + 36.0f >= HEIGHT){
            normal = getNormal((float2)(0.0f, 0.0f), (float2)(1.0f, 0.0f));
            newVelocity = velocity - 2 * (dot(velocity, normal)) * normal;
            p->velocity.x = newVelocity.x;
            p->velocity.y = newVelocity.y * 0.5f;
            p->position.x = pos.x;
            p->position.y = HEIGHT - 36.0f;
        }
    }
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

    checkParticleTriangleIntersection(p);
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
    if(!triangleCollision){
        if(id < 40){
            if(id < 20){
            inputData[id].velocity.x = 0.0f;
            inputData[id].velocity.y = 0.0f;

            inputData[id].position.x = 345.0f + id * 15.0f;
            inputData[id].position.y = 710.0f - id * 15.0f - id * 2.0f;

            }else{
                inputData[id].velocity.x = 0.0f;
                inputData[id].velocity.y = 0.0f;

                inputData[id].position.x = 935.0f - (40 - 1 - id) * 15.0f;
                inputData[id].position.y = 710.0f - (40 - 1 - id) * 15.0f - (40 - 1 - id) * 2.0f;
            }
        }else{
            float2 currVel = (float2)(inputData[id].velocity.x, inputData[id].velocity.y);
            float2 currVisF = (float2)(inputData[id].viscosityForce.x, inputData[id].viscosityForce.y);
            float2 currPreF = (float2)(inputData[id].pressureForce.x, inputData[id].pressureForce.y);
            float2 currPos = (float2)(inputData[id].position.x, inputData[id].position.y);
            float currDens = inputData[id].density;

            float2 newVel = currVel + (dt * (currVisF + currPreF + gravity * inputData[id].mass) / currDens);
            float2 newPos = currPos + dt * newVel;

            inputData[id].velocity.x = newVel.x;
            inputData[id].velocity.y = newVel.y;

            inputData[id].position.x = newPos.x;
            inputData[id].position.y = newPos.y;

            checkBounds(&inputData[id]);
            }
    }else{
            float2 currVel = (float2)(inputData[id].velocity.x, inputData[id].velocity.y);
            float2 currVisF = (float2)(inputData[id].viscosityForce.x, inputData[id].viscosityForce.y);
            float2 currPreF = (float2)(inputData[id].pressureForce.x, inputData[id].pressureForce.y);
            float2 currPos = (float2)(inputData[id].position.x, inputData[id].position.y);
            float currDens = inputData[id].density;

            float2 newVel = currVel + (dt * (currVisF + currPreF + gravity * inputData[id].mass) / currDens);
            float2 newPos = currPos + dt * newVel;

            inputData[id].velocity.x = newVel.x;
            inputData[id].velocity.y = newVel.y;

            inputData[id].position.x = newPos.x;
            inputData[id].position.y = newPos.y;

            checkBounds(&inputData[id]);
    }
}

__kernel void updateForces(__global ParticleData* inputData, __global int* size){
    calculateDensity(inputData, size);
    barrier(CLK_GLOBAL_MEM_FENCE);
    calculatePressureAndViscosityForce(inputData, size);
}

__kernel void updatePosition(__global ParticleData* inputData, __global int* size){
    move(inputData, size);
}