# AUTOMOC requires 2.8.6. The CMAKE_CXX_STANDARD variable requires 3.1.
cmake_minimum_required(VERSION 3.1)
project(morphologica)
set(VERSION 0.1)
set(SOVERSION 1.0)

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  (This can be changed with `cmake -DCMAKE_INSTALL_PREFIX=/some/place`")

# From CMAKE_SYSTEM work out which of __OSX__, __GLN__, __NIX__ or
# __WIN__ are required
message(STATUS "Operating system: " ${CMAKE_SYSTEM})
if(CMAKE_SYSTEM MATCHES Linux.*)
  set(MORPH_HOST_DEFINITION "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(MORPH_HOST_DEFINITION "-D__NIX__")
elseif(APPLE)
  set(MORPH_HOST_DEFINITION "-D__OSX__")
elseif(CMAKE_SYSTEM MATCHES Win.*)
  set(MORPH_HOST_DEFINITION "-D__WIN__")
else()
  message(ERROR " Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# An option that's useful for Ubuntu 16.04 builds
option(USE_GLEW "Link libglew.so (try if the linker can't find glCreateVertexArrays)" OFF)

# Using c++-14 (calls to std::decay_t in number_type.h). c++-17
# required to allow brace initializers for the morph::Vector class
set (CMAKE_CXX_STANDARD 17)

# Add the host definition to CXXFLAGS along with other switches
if (APPLE)
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -O3")
else()
  # To use Intel compiler, you can call cmake as: `cmake -DCMAKE_CXX_COMPILER=icpc ..` or `CXX=icpc cmake ..`
  if (CMAKE_CXX_COMPILER_ID MATCHES Intel)
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -std=c++17 -xHOST -O3 -D__ICC__")
  else () # GCC or Clang
    if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
      # Add compiler version check, to ensure gcc is version 7 or later.
      if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1)
        message(FATAL_ERROR "GCC version must be at least 7.1! Use CXX=g++-7 cmake (options) and don't forget to clean out your build directory first!")
      else()
        message(STATUS "GCC version ${CMAKE_CXX_COMPILER_VERSION} OK!")
      endif()
    endif()
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas -march=native -O3")
  endif()
endif()

# Tell clang to be quiet about brace initialisers
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")

# Additional GL compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")
if(USE_GLEW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GLEW")
endif(USE_GLEW)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
endif()

# The package managed version of armadillo8 on Ubuntu 18.04 LTS didn't
# work, but was required for OpenCV, so it's useful to be able to
# install a from-source build of Armadillo in /usr/local and link to
# this.
set(MORPH_ARMADILLO_LIBPATH "" CACHE PATH "The path to the armadillo library (e.g. /usr/local/lib)")

# A section to ensure library linking works on Apple. No longer necessary? Doesn't seem to be.
if(0)#APPLE
  set(CMAKE_MACOSX_RPATH ON)
  # use, i.e. don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)
  # when building, don't use the install RPATH already
  # (but later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
  # the RPATH to be used when installing
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  # the RPATH to be used when installing, but only if it's not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  endif("${isSystemDir}" STREQUAL "-1")
  include_directories(/usr/local/include)
endif()

# Lib finding
find_package(jsoncpp REQUIRED)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
if(USE_GLEW)
  find_package(GLEW REQUIRED)
endif(USE_GLEW)
# X11 is required only for morphdisplay0 on Apple, which uses Cocoa for glfw3/GL.
find_package(X11 REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(LAPACK REQUIRED)
# Find the HDF5 library. To prefer the use of static linking of HDF5, set HDF5_USE_STATIC_LIBRARIES first
#set(HDF5_USE_STATIC_LIBRARIES ON)
find_package(HDF5 REQUIRED)
find_package(Armadillo REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${X11_INC_SEARCH_PATH})
include_directories(${HDF5_INCLUDE_DIR})
# Two possible values for the armadillo include dirs
include_directories(${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS})
include_directories(${GLFW3_INCLUDE_DIR})

# rapidxml is bundled in the source, but its headers will be installed in ${CMAKE_INSTALL_PREFIX}/morph/, and they're symlinked in ${PROJECT_SOURCE_DIR}/morph
#include_directories("${PROJECT_SOURCE_DIR}/include/rapidxml-1.13")

# OpenGL code needs the local include directory, too
include_directories("${PROJECT_SOURCE_DIR}/include")

# Library code is compiled up as a shared library in lib/ (could also
# be compiled static if needed)
add_subdirectory(morph)

# Softmats add_subdirectory
add_subdirectory(softmats)

# We have to make a morph/rapidxml-1.13 installation for client code
# that uses the ReadCurves class.
add_subdirectory(include)

# Unit testing using the ctest framework
enable_testing()
add_subdirectory(tests)

# Example code (you can also see tests/ for examples)
add_subdirectory(examples)

# first we can indicate the documentation build as an option and set it to ON by default
option (BUILD_DOC "Build documentation" OFF)
if (BUILD_DOC)
  # check if Doxygen is installed
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc_doxygen ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
  else(DOXYGEN_FOUND)
    message("Need doxygen for documentation")
  endif(DOXYGEN_FOUND)
endif (BUILD_DOC)

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
