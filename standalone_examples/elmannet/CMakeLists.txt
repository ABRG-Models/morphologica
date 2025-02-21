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

# Flags common to all compilers
set(FLAGS_COMMON "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g")

# Add different optimizing flags depending on OS/Compiler
if (APPLE)
  set(CMAKE_CXX_FLAGS "${FLAGS_COMMON} -O3")
else()
  if (CMAKE_CXX_COMPILER_ID MATCHES Intel)
    set(CMAKE_CXX_FLAGS "${FLAGS_COMMON} -O3 -D__ICC__")
  else () # GCC or Clang
    # This assumes a gcc compiler (or a gcc mimic like Clang)
    set(CMAKE_CXX_FLAGS "${FLAGS_COMMON} -O3")
  endif()
endif()

# Tell clang to be quiet about brace initialisers:
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
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
