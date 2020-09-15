#
# This is an example CMakeLists.txt file to compile a program against
# libmorphologica (which must already have been built and installed).
#

# cmake version 3.1 provides the set(CMAKE_CXX_STANDARD 17) feature
cmake_minimum_required(VERSION 3.1)

# Give your project a name
project(RecurrentNet)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")

# From CMAKE_SYSTEM work out which of __OSX__, __GLN__, __NIX__ or __WIN__ are required
message(STATUS "Operating system: " ${CMAKE_SYSTEM})
if(CMAKE_SYSTEM MATCHES Linux.*)
  set(EXTRA_HOST_DEFINITION "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(EXTRA_HOST_DEFINITION "-D__NIX__")
elseif(APPLE)
  set(EXTRA_HOST_DEFINITION "-D__OSX__")
elseif(CMAKE_SYSTEM MATCHES Win.*)
  set(EXTRA_HOST_DEFINITION "-D__WIN__")
else()
  message(ERROR "Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# morphologica uses c++-17 language features
set(CMAKE_CXX_STANDARD 17)

# Common cflags
set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -Wno-sign-compare -g")

# OS dependent cflags
if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xpreprocessor")
else()
  # This assumes a gcc compiler (or a gcc mimic like Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Wno-unknown-pragmas -march=native -O3")
endif()

# Tell clang to be quiet about brace initialisers:
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

# Correct way to determine if OpenMP is available
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

# Additional GL compiler flags.
#
# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")
# Add GL defines to the CXX_FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# Find the libraries which will be needed
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(morphologica CONFIG REQUIRED)

add_executable(recurrentnet recurrentnet.cpp)
target_compile_definitions(recurrentnet PUBLIC FLT=float COMPILE_PLOTTING)
target_link_libraries(recurrentnet morphologica)
if(OpenMP_CXX_FOUND)
  target_link_libraries(recurrentnet PUBLIC OpenMP::OpenMP_CXX)
endif()

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
