cmake_minimum_required(VERSION 3.16)
project(SPHsim)

set(CMAKE_CXX_STANDARD 17)

include_directories("libs/SFML-2.5.1/include")
link_directories("libs/SFML-2.5.1/lib")

set(SOURCE_FILES main.cpp Particle.cpp Particle.h Boundaries.cpp Boundaries.h Constants.h)
add_executable(SPHsim ${SOURCE_FILES})

target_link_libraries(SPHsim sfml-graphics-d sfml-window-d sfml-system-d)