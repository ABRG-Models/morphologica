cmake_minimum_required(VERSION 3.10)
project(morphologica)
# Note that the project version is encoded in morph/version.h and not in this CMakeLists.txt

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
  message(FATAL_ERROR " Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# Need to find the host system processor to fine tune assembly in VisualFace.h
message(STATUS "Host system processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES aarch64)
  set(MORPH_HOST_DEFINITION "${MORPH_HOST_DEFINITION} -D__AARCH64__")
endif()

#
# The base C++ version in morphologica is current C++-17 but I am considering C++20 or 23
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckIncludeFileCXX) # CHECK_INCLUDE_FILE_CXX will be used in later scripts

# If I *require* std::format, I can start using it across morph. That
# limits us to gcc-13 on Ubuntu (though it's the libc++ that contains
# <format>). For now, test and use it only in examples.
check_include_file_cxx(format HAVE_STD_FORMAT)
if(NOT HAVE_STD_FORMAT)
  message(INFO " Your compiler does not appear to provide std::format, some examples won't build.")
  message(INFO " In future, <format> may be required across morphologica.")
endif()

# Add the host definition to CXXFLAGS along with other switches
if (APPLE)
  set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -O3")
else()
  if(NOT CMAKE_SYSTEM MATCHES Win.*)
    # To use Intel compiler, you can call cmake as: `cmake -DCMAKE_CXX_COMPILER=icpc ..` or `CXX=icpc cmake ..`
    if(CMAKE_CXX_COMPILER_ID MATCHES Intel)
      message(WARNING "Intel compiler has not been tested for some time")
      set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -Wall -g -std=c++20 -xHOST -O3 -D__ICC__")
    else() # GCC or Clang
      if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
        # Add compiler version check, to ensure gcc is version 11 or later.
        if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
          message(FATAL_ERROR "GCC version must be at least 11")
        else()
          message(STATUS "GCC version ${CMAKE_CXX_COMPILER_VERSION} OK!")
        endif()
      endif()
      set(COMPREHENSIVE_WARNING_FLAGS "-Wall -Wextra -Wpedantic -pedantic-errors -Werror -Wfatal-errors -Wno-psabi")
      set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -g ${COMPREHENSIVE_WARNING_FLAGS} -O3")
      if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
        # Make it possible to compile complex constexpr functions (gcc only)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-ops-limit=5000000000")
      else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-steps=500000000")
      endif()
    endif()
  else() # Windows
    # Set flags for Windows. /EHsc required for use of exceptions
    set(CMAKE_CXX_FLAGS "${MORPH_HOST_DEFINITION} -DNOMINMAX /EHsc /constexpr:steps500000000")
  endif() # CMAKE_SYSTEM MATCHES Win.*
endif()

# Some programs require the ability to define literal template args of
# type std::array (a C++-20 feature). Clang versions less than 18 can't.
set(ARRAY_LITERAL_TEMPL_ARGS_ARE_OK 0)
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18)
    set(ARRAY_LITERAL_TEMPL_ARGS_ARE_OK 1)
  endif()
else()
  set(ARRAY_LITERAL_TEMPL_ARGS_ARE_OK 1)
endif()

# Note that we require OpenMP version 3 or above for ability to use
# unsigned integral types in parallel for loops. At time of writing,
# Visual Studio 2019 offers OpenMP 2 by default and 3 with the special
# switch -openmp:llvm. I don't know how to get cmake to use the
# special version, so OpenMP will not work at present on
# Windows/VS2019. But then, you wouldn't use Windows for serious
# numerical work anyway.
find_package(OpenMP 3)
if(OpenMP_CXX_FOUND)
  message(STATUS "Found OpenMP version ${OpenMP_C_VERSION}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  if(APPLE)
    # FIXME: Check the OpenMP root environment variable, which is required on some Mac versions
  endif()
endif()

# Following `cmake --help-policy CMP0072`
set(OpenGL_GL_PREFERENCE "GLVND")

# Additional GL compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# We use GLAD headers. A multi-context (I think this for multiple
# windows with different OpenGL drivers) or single context (everything
# on the same GPU) header is available. The simpler single context is
# default, but the multi-context scheme may also be chosen.
option(USE_GLAD_MX "Use multi-context GLAD header morph/glad/gl_mx.h" OFF)
if(USE_GLAD_MX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GLAD_MX")
endif(USE_GLAD_MX)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
endif()

# The code in VisualFace which builds the Vera family truetype fonts
# into the program binary needs to have a define of MORPH_FONTS_DIR,
# so set it up here:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/fonts\\\"\"")
# Note that in client code you may have to do something similar. For
# example, if you're compiling with morphologica in a subdirectory, you may need:
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/morphologica/fonts\\\"\"")

# Lib finding
if(APPLE OR WIN32)
  find_package(OpenGL REQUIRED)
else()
  find_package(OpenGL REQUIRED EGL) # GL ES 3.1 used in one gl compute example
endif()

find_package(glfw3 3.2...3.4 REQUIRED)
# Find the HDF5 library. To prefer the use of static linking of HDF5, set HDF5_USE_STATIC_LIBRARIES first
#set(HDF5_USE_STATIC_LIBRARIES ON)
find_package(HDF5)
if(HDF5_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HDF5_DEFINITIONS}")
endif()
find_package(Armadillo)

include_directories(${OPENGL_INCLUDE_DIR})
if(HDF5_FOUND)
  include_directories(${HDF5_INCLUDE_DIR})
endif()
if(ARMADILLO_FOUND)
  # Two possible values for the armadillo include dirs
  include_directories(${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS})
endif()
include_directories(${GLFW3_INCLUDE_DIR})

# New, text rendering dependencies
find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

# Use packaged nlohmann json
find_package(nlohmann_json REQUIRED)

# If Qt5 is present, then some optional Qt examples will be compiled
find_package(Qt5 QUIET COMPONENTS Gui Core Widgets)
if(Qt5_FOUND)
  message(STATUS "Compiling Qt examples")
else()
  message(STATUS "NOT compiling Qt examples (Qt5 libraries not found)")
endif()


# wxwidgets. Optional. Use `apt install libwxgtk3.2-dev` on Debian/Ubuntu Linux (Min compatible Ubuntu version is 23.04)
set(WX_OPTIONAL_COMPONENTS "")
find_package(wxWidgets 3.1 COMPONENTS gl core base ${WX_OPTIONAL_COMPONENTS} net QUIET)
if(wxWidgets_FOUND)
  message(STATUS "Compiling wx examples")
else()
  message(STATUS "NOT compiling wx examples (no wxWidgets library found)")
endif()

# rapidxml is bundled in the source, but prefer packaged header
check_include_file_cxx ("rapidxml/rapidxml.hpp" HAVE_RAPIDXML)
if (NOT HAVE_RAPIDXML)
  include_directories("${PROJECT_SOURCE_DIR}/include/rapidxml-1.13")
endif() # else we have it in a directory included by default in the path

# All the morphologica headers are here
add_subdirectory(morph)

# Unit testing using the ctest framework
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif(BUILD_TESTS)

# incbin for Visual Studio; code-generation for colour tables
option(BUILD_UTILS "Build developer utils" OFF)
if(BUILD_UTILS)
  add_subdirectory(buildtools)
endif(BUILD_UTILS)

# Install the font files for program that need to to work with an
# *installed* morphologica, as opposed to an in-tree morphologica.
add_subdirectory(fonts)

# Example code (you can also see tests/ for examples)
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif(BUILD_EXAMPLES)

# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" OFF)
if(BUILD_DOC)
  # check if Doxygen is installed
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in)
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
endif(BUILD_DOC)

# For debugging of variables:
option(DEBUG_VARIABLES OFF)
if(DEBUG_VARIABLES)
  get_cmake_property(_variableNames VARIABLES)
  foreach(_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
  endforeach()
endif(DEBUG_VARIABLES)
