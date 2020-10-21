cmake_minimum_required(VERSION 3.16)
project(SPHsim)

set(CMAKE_CXX_STANDARD 17)

include_directories("libs/SFML-2.5.1/include")
link_directories("libs/SFML-2.5.1/lib")

link_directories("C:\\Program\ Files\ (x86)\\AMD APP SDK\\3.0\\lib\\x86_64")
include_directories("C:\\Program\ Files\ (x86)\\AMD APP SDK\\3.0\\include")

set(SOURCE_FILES src/main.cpp src/Particle.cpp src/Particle.h src/Boundaries.cpp src/Boundaries.h src/Constants.h src/opencl/OpenCLHelper.cpp src/opencl/OpenCLHelper.h src/ParticleData.h)
add_executable(SPHsim ${SOURCE_FILES})

target_link_libraries(SPHsim sfml-graphics-d sfml-window-d sfml-system-d)
target_link_libraries(SPHsim OpenCL)