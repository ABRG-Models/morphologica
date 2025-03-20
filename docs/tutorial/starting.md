---
title: Creating a new project
parent: Tutorials
layout: page
nav_order: 1
permalink: /tutorials/newproject
---
# Building your code with morphologica

This page will walk you through the process of setting up a new project to build morphologica code so that you can try the example code in the tutorials.

Because morphologica is header-only, it's very easy to work with. All you have to do is make sure your compiler knows where to find morphologica files, set a few flags in the compiler and link to a few essential libraries. The best way to coordinate all this is to use CMake from kitware, a widely used build management system.

## Dependencies

First, ensure you have the necessary dependencies installed. Classes in morphologica use Armadillo, OpenGL, Freetype, glfw3, nlohmann-json and HDF5. You won't necessarily need all of these; it depends on which classes you will use (see [here](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md#linking-a-morphologica-program) for details). For visualisation, you only need OpenGL, Freetype, nlohmann-json and glfw3. Platform-specific instructions can be found in the files [README.build.linux.md](https://github.com/ABRG-Models/morphologica/blob/main/README.build.linux.md), [README.build.mac.md](https://github.com/ABRG-Models/morphologica/blob/main/README.build.mac.md) and [README.build.windows.md](https://github.com/ABRG-Models/morphologica/blob/main/README.build.windows.md).

## Three necessities to build

Regardless of which build process you use (plain makefiles, autotools,
CMake or whatever), to build a program against morphologica, you need
to tell it: **1**) What compiler flags to add to the
compiler command line, including a directive to say where the fonts
that morphologica will compile into your binaries (if you're using
morph::Visual) are located. **2**) Where the morphologica headers are
to be found. **3**) which libraries to link to.

While you can install morphologica headers (and fonts) into a chosen
location (/usr/local by default) we recommend that you just clone a
copy of the morphologica repository into the base of your own source tree.

## Building with CMake

If you're using cmake, then this is what you add to your client code's
CMakeLists.txt:

### 1) Compiler flags

This piece of boiler-plate cmake will get you started with a sensible
set of compiler flags for morphologica:

```cmake
# morphologica uses c++-20 language features
set(CMAKE_CXX_STANDARD 20)

# Set up CMAKE_CXX_FLAGS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-g -O3 -Wall -Wfatal-errors -DGL_SILENCE_DEPRECATION")
elseif (WIN32)
  set(CMAKE_CXX_FLAGS "-DNOMINMAX /EHsc /Zc:__cplusplus")
else()
  # This assumes a gcc compiler (or a gcc mimic like Clang)
  set(CMAKE_CXX_FLAGS "-g -O3 -Wall -Wextra -Wpedantic -pedantic-errors -Werror -Wfatal-errors -Wno-psabi -Wno-unknown-pragmas")
endif()

# Add OpenMP flags here, if necessary
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/morphologica/fonts\\\"\"")
```

The last flag (**MORPH_FONTS_DIR**) helps your compiler to copy in the
fonts that morph::Visual needs.  If you are working with and
'installed' morphologica change this to:

```cmake
# Tell the program where the morph fonts are. Again, assuming you installed morphologica in /usr/local:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"/usr/local/share/fonts\\\"\"")
```

### 2) Include directories for headers

```cmake
# Find the libraries which will be needed
find_package(HDF5 REQUIRED)        # Only required if you used morph::HdfData
find_package(Armadillo REQUIRED)   # Only required if you use the Bezier curve classes or HexGrid/CartGrid
set(OpenGL_GL_PREFERENCE "GLVND")  # Write this line before find_package(OpenGL...
find_package(OpenGL REQUIRED)      # This, glfw3 and Freetype are required for morph::Visual
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(nlohmann_json REQUIRED)

# Define collections of includes for the dependencies
set(MORPH_INC_CORE ${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MORPH_INC_CORE} ${MORPH_INC_GL})

# MORPH_INCLUDE_PATH is set to the location at which the header directory 'morph' is found. For 'Installed morpholoigca':
set(MORPH_INCLUDE_PATH /usr/local CACHE PATH "The path to the morphologica headers (e.g. /usr/local/include or \$HOME/usr/include)")
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include)       # Allows morph/Header.h to be found
```
If you are working with in-tree morphologica, then replace the last section with:
```cmake
# Assuming that you installed morphologica in-tree (i.e. 'next to' schnakenberg.cpp).
set(MORPH_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/morphologica" CACHE PATH "The path to morphologica")
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include) # Allows GL3/gl3.h to be found
include_directories(BEFORE ${MORPH_INCLUDE_PATH})         # Allows morph/Header.h to be found
```

### 3) Links to dynamically linked libraries

Morphologica makes use of a number of libraries. Depending on which
classes you use from morphologica, you'll need to link to some or all
of these:

```cmake
set(MORPH_LIBS_CORE ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES})
set(MORPH_LIBS_GL OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(myprogtarget ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
```

### Example build files

You can find a complete example project called [morphologica_template](https://github.com/ABRG-Models/morphologica_template). This has a CMakeLists.txt file that you can copy and work from, along with a single example program (the graph again).

Additionally, each of the examples in [**morphologica/standalone_examples**](https://github.com/ABRG-Models/morphologica/tree/main/standalone_examples) has a CMakeLists.txt, written as if each
example was a standalone project in its own right.

The best example CMakeLists.txt file is the one in [**standalone_examples/schnakenberg**](https://github.com/ABRG-Models/morphologica/tree/main/standalone_examples/schnakenberg),
because it uses a broad range of morphologica's features.

In **standalone_examples/schnakenberg**, the default cmake build file, [**CMakeLists.txt**](https://github.com/ABRG-Models/morphologica/blob/main/standalone_examples/schnakenberg/CMakeLists.txt) assumes you did a
'morphologica install' into **/usr/local**, whereas
[**CMakeLists_intree.txt**](https://github.com/ABRG-Models/morphologica/blob/main/standalone_examples/schnakenberg/CMakeLists_intree.txt) will (if renamed to CMakeLists.txt) build the code assuming that you
placed a copy of the morphologica source  tree *inside* *standalone_examples/schnakenberg*.
