---
title: Orientation
parent: Welcome
layout: page
permalink: /orientation
nav_order: 5
---
This is an introduction to the morphologica repository; what's in each directory and why they are there.

First off, the library is header-only, which means that you don't need to compile it to include it in your programs. You just `#include` the relevant headers in your own `.cpp` files. All the header files that you need to include are in **morphologica/morph**.

However, there are many **test** and **example** programs that can be built, and some **standalone_examples** to help you to write CMake configurations that will build your program with morphologica.

At the time of writing, if you clone morphologica, change directory into its root and type `ls`, then you'll see something like this:

![An image of the following listing of directories and files:
boundaries              docs         README.build.linux.md     RRID.md              VisText.vert.glsl
build                   doxygen      README.build.mac.md       standalone_examples  Visual.frag.glsl
buildtools              examples     README.build.windows.md   tests                Visual.vert.glsl
cmake_cmd_windows.txt   fonts        README.cmake.md           triangles.frag
CMakeLists.txt          include      README.coding.md          triangles.vert
CMakeSettings.seb.json  LICENSE.txt  README.md                 valgrind.supp
Default.compute.glsl    morph        README.svg_boundaries.md  VisText.frag.glsl](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/morph_root.png?raw=true)

The files in the root directory include some markdown/readme files (*.md), some cmake configuration files (CMakeLists.txt, cmake_cmd_windows.txt, CMakeSettings.seb.json), the license file, a valgrind suppression file for debugging and a number of glsl files that probably belong elsewhere.

The sub-directories are:

* **boundaries/** Holds some example .svg drawings of boundaries that `morph::ReadCurves` can read
* **build/** The build directory. This won't exist if you just cloned, but you'll probably create it
* **buildtools/** Holds a tool called incbin which can be used when working on MS Windows
* **docs/** This documentation/reference website is in here and is automatically published on github pages
* **doxygen/** Holds a config file for doxygen code doc generation
* **examples/** All the example programs that demonstrate the use of the headers in `morph/`
* **fonts/** Holds a copy of the dejavu and ttf-bitstream-vera font files that morphologica builds into your binaries
* **include/** Some third party header code lives in here (GL/GL3, nlohmann for json, rapidxml and a header-version of the vera truetype font for Windows)
* **morph/** All the library headers are in here
* **standalone_examples/** Some examples of standalone projects that have their own CMake build processes. These give you a template to start working with morphologica
* **tests/** This contains test programs that can be run with `ctest` or `make test`.

## morph/

Most of morphologica is in the base namespace `morph` and their files are at the base of **morph/**. This is regardless of whether they are code for drawing visualizations or core maths code. Most classes are in a single `.h` header file with the same name as the class, although there are a few exceptions (such as **Random.h**, which contains several classes including ``morph::RandUniform`` and ``morph::RandNormal``).

Files called `XxxVisual.h` are classes that derive from `morph::VisualModel`. This means that they are models that you can incorporate into your `morph::Visual` scene. For example, `TriaxesVisual.h` is for drawing a 3D axis into which you can place a 3D scatter or quiver plot. `ColourbarVisual.h` helps you to draw a colour bar legend to display alongside one of your graphs.

Sub-directories in **morph/** are:

* **morph/bn/** Boolean net classes in namespace `morph::bn`
* **morph/gl/** Morphologica GL code (`morph::gl`), including code for running GL compute shaders
* **morph/linuxos/** A few utility functions for extracting info from a Linux computer (`morph::linuxos`)
* **morph/nn/** Neural net classes in namespace `morph::nn`
* **morph/qt/** Some classes to make it possible to incorporate morphologica OpenGL graphics in Qt programs (`morph::qt`)
* **morph/wx/** Classes to incorporate morphologica OpenGL graphics in wxWidgets and wxWindows programs (`morph::wx`)