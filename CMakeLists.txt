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

# An option to allow users without OpenMP (that's some Macs) to compile
option(COMPILE_WITH_OPENMP "Compile with OpenMP (adds -fopenmp or -qopenmp to compile lines, as appropriate)" ON)

# Add the host definition to CXXFLAGS along with other switches
if (APPLE)
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g")
    set(OPENMP_SWITCH "-Xpreprocessor -fopenmp")
else()
  # To use Intel compiler, you can call cmake as: `cmake -DCMAKE_CXX_COMPILER=icpc ..` or `CXX=icpc cmake ..`
  if (CMAKE_CXX_COMPILER_ID MATCHES Intel)
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -xHOST -O3 -D__ICC__")
    set(OPENMP_SWITCH "-qopenmp")
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
    set(OPENMP_SWITCH "-fopenmp")
  endif()
endif()

# Tell clang to be quiet about brace initialisers
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

if (COMPILE_WITH_OPENMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENMP_SWITCH}")
  if(APPLE)
    # On Apple, add the links for libomp:
    include_directories(/opt/local/include/libomp)
    link_libraries(-L/opt/local/lib/libomp omp)
  endif(APPLE)
endif (COMPILE_WITH_OPENMP)

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
if(USE_GLEW)
  find_package(GLEW REQUIRED)
endif(USE_GLEW)
find_package(X11 REQUIRED)
find_package(LAPACK REQUIRED)
# Find the HDF5 library. To prefer the use of static linking of HDF5, set HDF5_USE_STATIC_LIBRARIES first
find_package(HDF5 REQUIRED)
# pkgconfig is used to find JSON adn also to find lib paths for glfw3.
find_package(PkgConfig)

if(APPLE)
  # Manually set PKG_CONFIG_PATH on Mac, as mac ports will, by default, search only in /opt/local
  set(ENV{PKG_CONFIG_PATH}  "ENV{PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
endif(APPLE)

# JSON
if(${PKG_CONFIG_FOUND})
  message(STATUS "pkg-config was found. Search for JSON.")
  pkg_search_module(JSONCPP REQUIRED jsoncpp)
  if(${JSONCPP_FOUND})
    message(STATUS "pkg-config found jsoncpp. Excellent.")
  else(${JSONCPP_FOUND})
    message(WARNING ": jsoncpp was NOT found by pkg-config. Try default: /usr/local/lib/jsoncpp.a")
    set(JSONCPP_LDFLAGS "/usr/local/lib/libjsoncpp.a")
    set(JSONCPP_INCLUDE_DIRS "/usr/local/include")
  endif(${JSONCPP_FOUND})
else(${PKG_CONFIG_FOUND})
  message(STATUS "No pkg-config. Tryign default /usr/local/lib/libjsoncpp.a for jsoncpp.")
  set(JSONCPP_LDFLAGS "/usr/local/lib/libjsoncpp.a")
  set(JSONCPP_INCLUDE_DIRS "/usr/local/include")
endif(${PKG_CONFIG_FOUND})

# glfw3 provides a cmake find_package script. Because it doesn't set
# lib paths, we go on and use pkg-config to do this.
find_package(glfw3 QUIET)
if(${glfw3_FOUND})
  message(STATUS "cmake's find_package() found glfw3, using pkg-config to get details...")
  if(${PKG_CONFIG_FOUND})
    # Use pkg-config to check GLFW3 is present:
    pkg_search_module(GLFW REQUIRED glfw3)
    if(${GLFW_FOUND})
      message(STATUS "pkg-config found glfw(3). Excellent.")
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

include_directories(${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${X11_INC_SEARCH_PATH} ${HDF5_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${JSONCPP_INCLUDE_DIRS})

# rapidxml is bundled in the source:
include_directories("${PROJECT_SOURCE_DIR}/include/rapidxml-1.13")

# OpenGL code needs the local include directory, too
include_directories("${PROJECT_SOURCE_DIR}/include")

# Library code is compiled up as a shared library in lib/ (could also
# be compiled static if needed)
add_subdirectory(morph)

# We have to make a morph/rapidxml-1.13 installation for client code
# that uses the ReadCurves class.
add_subdirectory(include)

# Unit testing using the ctest framework
enable_testing()
add_subdirectory(tests)

# pkgconfig file installation
add_subdirectory(pc)

# Example code (you can also see tests/ for examples)
#add_subdirectory(examples)

# first we can indicate the documentation build as an option and set it to ON by default
option (BUILD_DOC "Build documentation" ON)
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
