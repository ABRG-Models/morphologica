#
# This is an example CMakeLists.txt file to compile a program against
# libmorphologica (which must already have been built and installed).
#

# cmake version 3.1 provides the set(CMAKE_CXX_STANDARD 17) feature
cmake_minimum_required(VERSION 3.1)

# Give your project a name
project(NeuralNet)

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
  set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g -Xpreprocessor -fopenmp")
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
find_package(GLUT REQUIRED)
find_package(X11 REQUIRED)

# Find HDF5 and jsoncpp by different methods on Apple and Linux:
if(APPLE)
  find_package (HDF5 REQUIRED)
  set (HDF5LIBLINK ${HDF5_C_LIBRARY_hdf5})
  set (JSONLIBLINK /usr/local/lib/libjsoncpp.dylib)
  # On Apple, add the links for libomp:
  include_directories(/opt/local/include/libomp)
  link_libraries(-L/opt/local/lib/libomp omp)
else()
  set (LIB_TYPE SHARED) # STATIC or SHARED
  string(TOLOWER ${LIB_TYPE} SEARCH_TYPE)
  find_package (HDF5 NAMES hdf5 COMPONENTS C ${SEARCH_TYPE} REQUIRED)
endif(APPLE)

# glfw3 library finding. However, it's possible to compile from
# source, and there are instructions on the glfw3 website on how to do
# this with cmake.
#
# glfw3 provides a cmake find_package script, though it doesn't set
# lib paths etc. Then you just link with "glfw3" if you happened to
# compile the static libglfw3.a or with "glfw" if you compiled the
# dynamic libglfw.so
find_package(glfw3 QUIET)
# You can find GLFW library with pkgconfig, but it's switched off for now
if(0)
  find_package(PkgConfig)
  if(PKG_CONFIG_FOUND)
    # Use pkg-config to check GLFW3 is present:
    pkg_check_modules(GLFW3 REQUIRED glfw3)
    if (GLFW3_FOUND)
      # Good....
      message(INFO, "pkg-config found glfw3. Excellent.")
    else(GLFW3_FOUND)
      # Bad....
      message(ERROR, "glfw3 was NOT found by pkg-config. Uh oh.")
    endif(GLFW3_FOUND)
  else()
    message(WARNING "There's no pkg-config on this system to check for libglfw3. You may need to try `sudo apt-get install pkg-config`")
  endif()
endif()

include_directories(${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${X11_INC_SEARCH_PATH} ${HDF5_INCLUDE_DIR})

# Find libmorphologica using pkgconfig
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  message(STATUS "Have pkg_config, searching for libmorphologica...")

  if(APPLE)
    # Manually set PKG_CONFIG_PATH on Mac, as mac ports with by
    # default search only in /opt/local
    set(ENV{PKG_CONFIG_PATH}  "ENV{PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/lib/pkgconfig")
  endif(APPLE)

  pkg_check_modules(MORPH REQUIRED libmorphologica)
  if (MORPH_FOUND)
    message(STATUS "pkg_config found libmorphologica")
    include_directories(${MORPH_INCLUDEDIR})
    include_directories(${MORPH_INCLUDEDIR}/morph)
    message(STATUS "Included morph includedir:  ${MORPH_INCLUDEDIR} and ${MORPH_INCLUDEDIR}/morph")
  else()
    message(FATAL_ERROR "You need libmorphologica from github.com/ABRG_Models/morphologica")
  endif(MORPH_FOUND)
else()
  message(WARNING "There's no pkg-config on this system to check for libmorphologica. "
    "You can set MORPH_LIB with `cmake -DMORPH_LIB=/pathto/lib` ...")
endif()

# On Apple, have to explicitly add the X11 library directory
if(APPLE)
  link_directories(/usr/X11R6/lib)
endif()

#
# All the library finding is now done. It's time to compile the program.
#
if(${glfw3_FOUND}) # This example *requires* the modern OpenGL code
  # Schnakenberg RD system
  add_executable(nn1 nn1.cpp)
  target_compile_definitions(nn1 PUBLIC FLT=float COMPILE_PLOTTING)
  add_executable(nn2 nn2.cpp)
  target_compile_definitions(nn2 PUBLIC FLT=float COMPILE_PLOTTING)
  add_executable(nn3 nn3.cpp)
  target_compile_definitions(nn3 PUBLIC FLT=float COMPILE_PLOTTING)

  if(APPLE)
    target_link_libraries(nn1 ${OpenCV_LIBS} ${MORPH_LDFLAGS} ${GLFW_LIBRARIES} glfw ${OPENGL_LIBRARIES})
  else()
    target_link_libraries(nn1 ${MORPH_LDFLAGS} armadillo ${OPENGL_LIBRARIES})
  endif()
  target_link_libraries(nn2 ${MORPH_LDFLAGS} armadillo ${OPENGL_LIBRARIES})
  target_link_libraries(nn3 ${MORPH_LDFLAGS} armadillo ${OPENGL_LIBRARIES})

else()
    message(FATAL_ERROR "You need libglfw3 to compile this program!")
endif()

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
