cmake_minimum_required(VERSION 3.25)
project(tifo_project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -g -std=c++23 -lm -m64 -march=native")

# Set path to Qt
list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt@5")

# Handle Qt moc preprocessor automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt packages
find_package(Qt5 COMPONENTS Widgets REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hh")

add_executable(tifo_project main.cpp ${SOURCES} src/main_window.hh src/image_to_qt.cc src/image_to_qt.hh)

# Link Qt to your project
target_link_libraries(tifo_project Qt5::Widgets)
