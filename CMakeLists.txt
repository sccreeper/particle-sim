cmake_minimum_required(VERSION 3.15)
project(particle_sim)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

find_program(IWYU_EXE NAMES include-what-you-use iwyu)
if(IWYU_EXE)
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU_EXE})
else()
    message(STATUS "include-what-you-use not found, skipping IWYU checks")
endif()

include(FetchContent)

set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
set(PLATFORM "Desktop" CACHE STRING "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

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

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wuninitialized -Wmaybe-uninitialized)