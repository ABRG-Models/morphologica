# AUTOMOC requires 2.8.6. Per-target include directories would require 2.8.8
cmake_minimum_required(VERSION 2.8.8)
project(morphologica)
set(VERSION 0.1)
set(SOVERSION 1.0)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")

set(CMAKE_CXX_FLAGS "-Wall -std=c++11")

# Lib finding
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(X11 REQUIRED)

include_directories(${OPENCV_INCLUDE_DIR} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

# rapidxml is bundled in the source:
#include_directories ("${PROJECT_SOURCE_DIR}/include/rapidxml-1.13")

# Library code is compiled up as a shared library in lib/ (could also be compiled static if needed)
add_subdirectory(src)

# Unit testing using the ctest framework
enable_testing()
add_subdirectory(tests)

# pkgconfig file installation
add_subdirectory(pc)

# For debugging of variables:
set(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
