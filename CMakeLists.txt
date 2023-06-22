cmake_minimum_required(VERSION 3.25)
project(tifo_project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -g -std=c++23 -lm -m64 -march=native")

file(GLOB_RECURSE SOURCES "src/*.cc")
file(GLOB_RECURSE HEADERS "src/*.hh")

add_executable(tifo_project main.cpp ${SOURCES})