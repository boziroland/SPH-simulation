cmake_minimum_required(VERSION 3.16)
project(SPHsim)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message("debug mode")
else (CMAKE_BUILD_TYPE MATCHES Release)
    message("release mode")
endif ()

include_directories("libs/SFML-2.5.1/include")
link_directories("libs/SFML-2.5.1/lib")

link_directories("C:\\Program\ Files\ (x86)\\AMD APP SDK\\3.0\\lib\\x86_64")
include_directories("C:\\Program\ Files\ (x86)\\AMD APP SDK\\3.0\\include")

set(SOURCE_FILES src/main.cpp src/Particle.cpp src/Particle.h src/Boundaries.cpp src/Boundaries.h src/Constants.h src/opencl/OpenCLHelper.cpp src/opencl/OpenCLHelper.h src/ParticleData.h src/Timer.h)
add_executable(SPHsim ${SOURCE_FILES})

if (CMAKE_BUILD_TYPE MATCHES Debug)
    target_link_libraries(SPHsim debug sfml-graphics-d debug sfml-window-d debug sfml-system-d)
else (CMAKE_BUILD_TYPE MATCHES Release)
    target_link_libraries(SPHsim optimized sfml-graphics optimized sfml-window optimized sfml-system)
endif ()

target_link_libraries(SPHsim OpenCL)