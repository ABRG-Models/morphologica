# AUTOMOC requires 2.8.6. Per-target include directories would require 2.8.8
cmake_minimum_required(VERSION 2.8.8)
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
  message(ERROR "Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# Add the host definition to CFLAGS
set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION}")
set(CMAKE_C_FLAGS "${MORPH_HOST_DEFINITION}")

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c++11")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -g -std=c++11")
endif()

# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")

# Additional GL compiler flags.
# maybe -DGL_GLEXT_PROTOTYPES
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# The package managed version of armadillo8 on Ubuntu 18.04 LTS didn't
# work, but was required for OpenCV, so it's useful to be able to
# install a from-source build of Armadillo in /usr/local and link to
# this.
set(MORPH_ARMADILLO_LIBPATH "" CACHE PATH "The path to the armadillo library (e.g. /usr/local/lib)")

# A section to ensure library linking works on Apple.
if(APPLE)
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
endif(APPLE)

# Lib finding
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(X11 REQUIRED)
find_package(LAPACK REQUIRED)
# Find the HDF5 library. To prefer the use of static linking of HDF5, set HDF5_USE_STATIC_LIBRARIES first
find_package(HDF5 REQUIRED)
# pkgconfig is used to find JSON adn also to find lib paths for glfw3.
find_package(PkgConfig)

# JSON
if(${PKG_CONFIG_FOUND})
  message(INFO ": pkg-config was found. Search for JSON.")
  pkg_search_module(JSONCPP REQUIRED jsoncpp)
  if(${JSONCPP_FOUND})
    message(INFO ": pkg-config found jsoncpp. Excellent.")
  else(${JSONCPP_FOUND})
    message(WARNING ": jsoncpp was NOT found by pkg-config. Try default: /usr/local/lib/jsoncpp.a")
    set(JSONCPP_LDFLAGS "/usr/local/lib/libjsoncpp.a")
  endif(${JSONCPP_FOUND})
else(${PKG_CONFIG_FOUND})
  message(INFO ": No pkg-config. Tryign default /usr/local/lib/libjsoncpp.a for jsoncpp.")
  set(JSONCPP_LDFLAGS "/usr/local/lib/libjsoncpp.a")
endif(${PKG_CONFIG_FOUND})

# glfw3 provides a cmake find_package script. Because it doesn't set
# lib paths, we go on and use pkg-config to do this.
find_package(glfw3 QUIET)
if(${glfw3_FOUND})
  message(INFO ": cmake's find_package() found glfw3, using pkg-config to get details...")
  if(${PKG_CONFIG_FOUND})
    # Use pkg-config to check GLFW3 is present:
    pkg_search_module(GLFW REQUIRED glfw3)
    if(${GLFW_FOUND})
      message(INFO ": pkg-config found glfw(3). Excellent.")
    else(${GLFW_FOUND})
      message(ERROR ": glfw(3) was NOT found by pkg-config. Uh oh.")
    endif(${GLFW_FOUND})
  else(${PKG_CONFIG_FOUND})
    message(WARNING "There's no pkg-config on this system to find exact details for glfw3. You may need to try `sudo apt-get install pkg-config` or similar")
  endif(${PKG_CONFIG_FOUND})
endif(${glfw3_FOUND})

# Armadillo
if(APPLE)
  # Give a hint on where to find Armadillo on Mac
  find_package(Armadillo 9.20 HINTS /opt/local/lib REQUIRED)
else(APPLE)
  # A simpler find package for armadillo on non-mac:
  find_package(Armadillo REQUIRED)
endif(APPLE)

include_directories(${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${X11_INC_SEARCH_PATH} ${HDF5_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR})

# rapidxml is bundled in the source:
include_directories("${PROJECT_SOURCE_DIR}/include/rapidxml-1.13")

# OpenGL code needs the local include directory, too
include_directories("${PROJECT_SOURCE_DIR}/include")

# Library code is compiled up as a shared library in lib/ (could also
# be compiled static if needed)
add_subdirectory(src)

# We have to make a morph/rapidxml-1.13 installation for client code
# that uses the ReadCurves class.
add_subdirectory(include)

# Unit testing using the ctest framework
enable_testing()
add_subdirectory(tests)

# pkgconfig file installation
add_subdirectory(pc)

# For debugging of variables:
set(DEBUG_VARIABLES ON)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
