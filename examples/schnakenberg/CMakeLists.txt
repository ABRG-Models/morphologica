#
# This is an example CMakeLists.txt file to compile a program against
# libmorphologica (which must already have been built and installed).
#

# cmake version 3.1 provides the set(CMAKE_CXX_STANDARD 17) feature
cmake_minimum_required(VERSION 3.1)

# Give your project a name
project(Schnakenberg)

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

# Add the host definition to CXXFLAGS along with other switches, depending on OS/Compiler
if (APPLE)
  set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g -Xpreprocessor")
else()
  # This assumes a gcc compiler (or a gcc mimic like Clang)
  set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g -Wno-unused-result -Wno-unknown-pragmas -march=native -O3 -fopenmp")
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

# Find the libraries which will be needed
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Oddly, the build succeeds (at least on Mac) without
# find_package(Armadillo) - the link to the library is passed through
# with morphologica-config.cmake, unlike all the others. I think it's
# down to differences in the ways that packages create their
# package-config.cmake files.
if(0)
  find_package(Armadillo REQUIRED)
endif(0)

# Find HDF5 and jsoncpp by different methods on Apple and Linux:
if(APPLE)
  find_package (HDF5 REQUIRED)
  # On Apple, possibly add the links for libomp if you have it:
  #include_directories(/opt/local/include/libomp)
  #link_libraries(-L/opt/local/lib/libomp omp)
else()
  # I found this slightly more complex call to find_package to be
  # necessary/favourable at some point. Must test to see if this is
  # still the case.
  find_package (HDF5 NAMES hdf5 COMPONENTS C shared REQUIRED)
endif(APPLE)

# The config keyword means find the morphologica that was compiled and
# installed on this computer, i.e. it has configured locations for
# library paths.
find_package(morphologica CONFIG REQUIRED)

#
# All the library finding is now done. It's time to compile the program.
#
# The Schnakenberg RD system is compiled from a single c++ file
add_executable(schnakenberg schnakenberg.cpp)
# Set compiler definitions. The size of our floats and the option to
# compile in the realtime visualization.
target_compile_definitions(schnakenberg PUBLIC FLT=float COMPILE_PLOTTING)
# Only one link is mentioned - to morphologica. Other links are pulled
# in from morphologica-config.cmake.
target_link_libraries(schnakenberg morphologica)

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
