#
# This is an example CMakeLists.txt file to compile a morphologica program
#

cmake_minimum_required(VERSION 3.10)

# Give your project a name
project(ElmanNet)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")

# morphologica uses c++-20 language features
set(CMAKE_CXX_STANDARD 20)

# Add different optimizing flags depending on OS/Compiler
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wall -Wfatal-errors -g -O3")
elseif (WIN32)
  set(CMAKE_CXX_FLAGS "-DNOMINMAX /EHsc")
else()
  if (CMAKE_CXX_COMPILER_ID MATCHES Intel)
    set(CMAKE_CXX_FLAGS "-Wall -Wfatal-errors -g -O3")
  else () # GCC or Clang
    # This assumes a gcc compiler (or a gcc mimic like Clang)
    set(CMAKE_CXX_FLAGS "-Wall -Wfatal-errors -g -O3")
  endif()
endif()

# Additional GL compiler flags.
#
# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")
# Add GL defines to the CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# We assume that the morphologica tree is 'next to' our .cpp files.
set(MORPH_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/morphologica" CACHE PATH "The path to morphologica")
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include) # Allows GL3/gl3.h to be found
include_directories(BEFORE ${MORPH_INCLUDE_PATH})         # Allows morph/Header.h to be found

# A single executable, and no links are required for this program.
add_executable(elman1 elman1.cpp)

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
