cmake_minimum_required(VERSION 3.16)
project(SPHsim)

set(CMAKE_CXX_STANDARD 17)

include_directories("libs/SFML-2.5.1/include")
link_directories("libs/SFML-2.5.1/lib")

set(SOURCE_FILES src/main.cpp src/Particle.cpp src/Particle.h src/Boundaries.cpp src/Boundaries.h src/Constants.h)
add_executable(SPHsim ${SOURCE_FILES})

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
endif ()

target_link_libraries(SPHsim sfml-graphics-d sfml-window-d sfml-system-d)