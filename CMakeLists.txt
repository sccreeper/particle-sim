cmake_minimum_required(VERSION 3.15)
project(particle_sim)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

include(FetchContent)

set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(PLATFORM "SDL" CACHE STRING "" FORCE)

set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(raylib)

add_executable(${PROJECT_NAME} sim/main.cpp
    sim/materials.hpp
    sim/registry.hpp
    sim/simulation.hpp
    sim/simulation.cpp
    sim/utils.hpp
    sim/utils.cpp
    sim/materials.cpp
    sim/logging.hpp
    sim/logging.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)