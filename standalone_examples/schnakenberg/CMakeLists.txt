#
# This is an example CMakeLists.txt file to compile a program against
# a morphologica that is 'in-tree'
#

# cmake version 3.1 provides the set(CMAKE_CXX_STANDARD 17) feature
cmake_minimum_required(VERSION 3.1)

# Give your project a name
project(Schnakenberg)

# From CMAKE_SYSTEM work out which of __OSX__, __GLN__, __NIX__ are required
if(CMAKE_SYSTEM MATCHES Linux.*)
  set(OS_FLAG "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(OS_FLAG "-D__NIX__")
elseif(APPLE)
  set(OS_FLAG "-D__OSX__")
else()
  message(ERROR "Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# morphologica uses c++-17 language features
set(CMAKE_CXX_STANDARD 17)

# Add the host definition to CXXFLAGS along with other switches, depending on OS/Compiler
if (APPLE)
  set(CMAKE_CXX_FLAGS "${OS_FLAG} -Wall -Wfatal-errors -g -O3")
else()
  # This assumes a gcc compiler (or a gcc mimic like Clang)
  set(CMAKE_CXX_FLAGS "${OS_FLAG} -Wall -Wfatal-errors -g -O3 -Wno-unused-result -Wno-unknown-pragmas -march=native")
endif()

# Tell clang to be quiet about brace initialisers:
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

# Add OpenMP flags here, if necessary
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Additional GL compiler flags.
#
set(OpenGL_GL_PREFERENCE "GLVND") # Following `cmake --help-policy CMP0072`
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
endif()

# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/morphologica/fonts\\\"\"")

# Find the libraries which will be needed
find_package(HDF5 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define collections of includes that have to be made for morphologica
set(MORPH_INC_CORE ${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MORPH_INC_CORE} ${MORPH_INC_GL})
# Assuming that you installed morphologica in-tree (i.e. 'next to' schnakenberg.cpp).
set(MORPH_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/morphologica" CACHE PATH "The path to morphologica")
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include) # Allows GL3/gl3.h to be found
include_directories(BEFORE ${MORPH_INCLUDE_PATH})         # Allows morph/Header.h to be found

#
# All the library finding is now done. It's time to compile the program.
#

# The Schnakenberg RD system is compiled from a single c++ file
add_executable(schnakenberg schnakenberg.cpp)
# Set compiler definitions. The size of our floats and the option to compile in the realtime visualization.
target_compile_definitions(schnakenberg PUBLIC FLT=float COMPILE_PLOTTING)

# Morphologica code requires a number of libraries, collected into 'CORE' and 'GL'.
set(MORPH_LIBS_CORE ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES})
set(MORPH_LIBS_GL OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(schnakenberg ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})

if(OpenMP_FOUND)
  target_link_libraries(schnakenberg OpenMP::OpenMP_CXX)
endif()

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
