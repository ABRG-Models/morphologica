# morphologica

![A banner image of a hexgrid surface plot](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/banner.png?raw=true)

Library code used in models developed by Stuart P. Wilson, Seb James
and co-workers in the Wilson Lab.

You'll find all in the **code** in the [**morph**](https://github.com/ABRG-Models/morphologica/tree/main/morph) directory. For **example code** and **screenshots**, see [this page](https://github.com/ABRG-Models/morphologica/tree/main/examples).

morphologica has some **demo/tutorial** content on YouTube: https://www.youtube.com/playlist?list=PLwiQ_IuTOr_Us9_tBde96VLYQlRWOYeAP

This header-only C++ code provides **simulation support facilities** for our simulations of
dynamical systems.

It helps with:

* **Visualizing your model while it runs**. A modern OpenGL visualization
  scheme called **[morph::Visual](https://github.com/ABRG-Models/morphologica/blob/main/morph/Visual.h)** 
  provides the ability to visualise 2D and 3D graphs
  of surfaces, lines, bars, scatter plots and quiver plots with minimal
  processing overhead. Here's a [morph::Visual helloworld](https://github.com/ABRG-Models/morphologica/blob/main/examples/helloworld.cpp) and [a more complete example](https://github.com/ABRG-Models/morphologica/blob/main/examples/visual.cpp). It's almost as easy to [draw a graph in C++ with morphologica](https://github.com/ABRG-Models/morphologica/blob/main/examples/graph1.cpp) as it is to do so [in Python](https://github.com/ABRG-Models/morphologica/blob/main/examples/graph1.py).

* **Configuration**: morphologica allows you to easily set up a simulation
  parameter configuration system, using the JSON reading and writing
  abilities of **[morph::Config](https://github.com/ABRG-Models/morphologica/blob/main/morph/Config.h)**. ([morph::Config Example](https://github.com/ABRG-Models/morphologica/blob/main/examples/jsonconfig.cpp))

* **Saving data from your simulation**. morphologica provides a set of
  easy-to-use convenience wrappers (**[morph::HdfData](https://github.com/ABRG-Models/morphologica/blob/main/morph/HdfData.h)**) around the HDF5 C
  API. By saving data in a standard format, it is easy to access
  simulation data in python, MATLAB or Octave for analysis and graphing. ([HdfData Example](https://github.com/ABRG-Models/morphologica/blob/main/examples/hdfdata.cpp))

It keeps *out of the way* of what kind of simulation you write. Our
programs typically start with some kind of preamble, in which we use
morph::Config to load up a JSON parameter file defining the values of
the parameters for the simulation run. We might also use
morph::HdfData to retrieve some data (e.g. the state) from an earlier
simulation and then set up a morph::Visual object for the
visualization. We then might call a function, or create a class object
which defines the simulation. *This may or may not access features
from the morphologica headers*.

As the simulation progresses, we update the data in the morph::Visual
scene; save images from the scene for movie making and record data as
often as we want it using morph::HdfData. At the end of the program,
as well as saving any final data, we use morph::Config to save out a
'version of record' of the parameters that were used, along with git
information which morph::Config can extract so that we could find the
exact version of the simulation for future reproduction of the result.

![Shows a variety of visualisations created with morphologica](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/examples.png?raw=true)

*A selection of visualisations made with morphologica. **A** 2D graphs. **B** A self-organising map simulation (orientation preference maps). **C** Three dimensional quiver plot. **D** gene driven reaction diffusion model. **E** Debugging a large model.*

Although it need not be incorporated into your actual simulation,
morphologica does also provide classes that you might find
useful. Examples include:

* **[morph::HexGrid](https://github.com/ABRG-Models/morphologica/blob/main/morph/HexGrid.h)** and **[morph::CartGrid](https://github.com/ABRG-Models/morphologica/blob/main/morph/CartGrid.h)**: classes for running simulations on hexagonal or Cartesian
grids (managing all the neighbour relationships between elements and
allowing you to specific various boundary shapes for your domain)

* **[morph::Vector](https://github.com/ABRG-Models/morphologica/blob/main/morph/Vector.h)** and **[morph::vVector](https://github.com/ABRG-Models/morphologica/blob/main/morph/vVector.h)**: Cool mathematical vector classes - these are like std::vector and std::array but they also sport maths methods. [vVector usage example](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvVector.cpp).

* **[morph::MathAlgo](https://github.com/ABRG-Models/morphologica/blob/main/morph/MathAlgo.h)** a class containing mathematical algorithms.

* **[morph::BezCurve](https://github.com/ABRG-Models/morphologica/blob/main/morph/BezCurve.h)** and friends: classes for working with Bezier
    curves.

* **[morph::Winder](https://github.com/ABRG-Models/morphologica/blob/main/morph/Winder.h)** A class to compute the winding number of a path.

* **[morph::Scale](https://github.com/ABRG-Models/morphologica/blob/main/morph/Scale.h)** A class for simple scaling/transformation of numbers.

* **[morph::NM_Simplex](https://github.com/ABRG-Models/morphologica/blob/main/morph/NM_Simplex.h)** and **[morph::Anneal](https://github.com/ABRG-Models/morphologica/blob/main/morph/Anneal.h)** Optimization algorithms. Example [simulated annealing usage](https://github.com/ABRG-Models/morphologica/blob/main/examples/anneal_asa.cpp#L162) and the [Nelder-Mead simplex method](https://github.com/ABRG-Models/morphologica/blob/main/examples/rosenbrock.cpp#L97)

* **[morph::RandUniform](https://github.com/ABRG-Models/morphologica/blob/main/morph/Random.h)** and friends. Wrapper classes around
    C++'s high quality random number generation code ([Usage example](https://github.com/ABRG-Models/morphologica/blob/main/examples/randvec.cpp#L22)).

* **[morph::ReadCurves](https://github.com/ABRG-Models/morphologica/blob/main/morph/ReadCurves.h)** Code to read SVG drawings to turn Bezier-curve
    based lines into paths containing evenly spaced coordinates.

morphologica is a way of storing our 'group knowledge' for posterity.

Some existing projects which use morphologica are:
* **BarrelEmerge** A reaction-diffusion style model: https://github.com/ABRG-Models/BarrelEmerge
* **RetinoTectal** Reaction-diffusion and agent-based modelling: https://github.com/sebjameswml/RetinoTectal
* **ArtificialGeneNets** Neural networks: https://github.com/stuartwilson/ArtificialGeneNets

## Code documentation

morphologica code is enclosed in the **morph** namespace. See the header files (They're all in [morph/](https://github.com/ABRG-Models/morphologica/tree/main/morph)) for the code documentation.

You can find example programs which are compiled when you do the standard
cmake-driven build of morphologica in both the [tests/](https://github.com/ABRG-Models/morphologica/tree/main/tests) subdirectory
and the [examples/](https://github.com/ABRG-Models/morphologica/tree/main/examples) subdirectory. The readme in examples/ has some nice
screenshots.

For full, compilable, standalone examples of the code, see the
standalone_examples/ subdirectory. Use these as templates for creating
your own projects which use morphologica library code.

See [README.coding.md](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md) for a quick-start guide to the main classes.

## Building code against morphologica

First, ensure you have the necessary dependencies installed. Classes in morphologica use Armadillo, OpenGL, Freetype, glfw3, jsoncpp and HDF5. You won't necessarily need all of these; it depends on which classes you will use (see [here](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md#linking-a-morphologica-program) for details). For visualisation, you only need OpenGL, Freetype, glfw3 and jsoncpp. Platform-specific instructions can be found in the files [README.install.linux.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.linux.md), [README.install.mac.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.mac.md) and [README.install.windows.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.windows.md).

To build a program against morphologica, you need to tell your build process: **1**) What compiler
flags to add to the compiler command line, including a directive to
say where the fonts that
morphologica will compile into your binaries (if you're using
morph::Visual) are located. **2**) Where the
morphologica headers are to be found. **3**) which libraries to link to.

You can either build with morphologica headers (and fonts) installed
in your chosen location (/usr/local by default) *or* you can just
clone a copy of morphologica source into your own source tree.

(Seb prefers this second option: 'clone and go'. Just git clone
morphologica into your source tree (or somewhere else) and then make
sure your compiler can find the includes, fonts and libraries.)

Depending on the option you choose, small differences to the paths
used for **1) Compiler flags** and **2) Include directories** will be
required.

If you're using cmake, then this is what you add to your client code's
CMakeLists.txt:

### 1) Compiler flags

This piece of boiler-plate cmake will get you started with a sensible
set of compiler flags for morphologica:

```cmake
# From CMAKE_SYSTEM work out which of __OSX__, __GLN__, __NIX__ or __WIN__ are required
message(STATUS "Operating system: " ${CMAKE_SYSTEM})
if(CMAKE_SYSTEM MATCHES Linux.*)
  set(EXTRA_HOST_DEFINITION "-D__GLN__")
elseif(CMAKE_SYSTEM MATCHES BSD.*)
  set(EXTRA_HOST_DEFINITION "-D__NIX__")
elseif(APPLE)
  set(EXTRA_HOST_DEFINITION "-D__OSX__")
else()
  message(ERROR "Operating system not supported: " ${CMAKE_SYSTEM})
endif()

# morphologica uses c++-17 language features
set(CMAKE_CXX_STANDARD 17)

# Add the host definition to CXXFLAGS along with other switches,
# depending on OS/Compiler and your needs/preferences
if (APPLE)
  set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g -O3")
else() # assume g++ (or a gcc/g++ mimic like Clang)
  set(CMAKE_CXX_FLAGS "${EXTRA_HOST_DEFINITION} -Wall -Wfatal-errors -g -Wno-unused-result -Wno-unknown-pragmas -march=native -O3")
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
set(OpenGL_GL_PREFERENCE "GLVND")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")
if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION")
endif()

# Tell the program where the morph fonts are. Again, assuming you installed morphologica in /usr/local:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"/usr/local/share/fonts\\\"\"")
```

The last flag (**MORPH_FONTS_DIR**) helps your compiler to copy in the
fonts that morph::Visual needs.  If you are working with 'in-tree'
morphologica change this to:

```cmake
# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/morphologica/fonts\\\"\"")
```

### 2) Include directories for headers

```cmake
# Find the libraries which will be needed
find_package(jsoncpp REQUIRED)     # Required for morph::Config and morph::Visual
find_package(HDF5 REQUIRED)        # Only required if you used morph::HdfData
find_package(Armadillo REQUIRED)   # Only required if you use the Bezier curve classes
find_package(OpenGL REQUIRED)      # This, glfw3 and Freetype are required for morph::Visual
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define collections of includes for the dependencies
get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
set(MORPH_INC_CORE ${JSON_INC_PATH} ${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MORPH_INC_CORE} ${MORPH_INC_GL})

# MORPH_INCLUDE_PATH is set to the location at which the header directory 'morph' is found. For 'Installed morpholoigca':
set(MORPH_INCLUDE_PATH /usr/local CACHE PATH "The path to the morphologica headers (e.g. /usr/local/include or \$HOME/usr/include)")
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include/morph) # Allows GL3/gl3.h to be found
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
set(MORPH_LIBS_CORE ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} jsoncpp_lib)
set(MORPH_LIBS_GL OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(myprogtarget ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
```

### Example build files

Each of the examples in [**morphologica/standalone_examples**](https://github.com/ABRG-Models/morphologica/tree/main/standalone_examples) has a CMakeLists.txt, written as if each
example was a standalone project in its own right.

The best example CMakeLists.txt file is the one in [**standalone_examples/schnakenberg**](https://github.com/ABRG-Models/morphologica/tree/main/standalone_examples/schnakenberg),
because it uses a broad range of morphologica's features.

In **standalone_examples/schnakenberg**, the default cmake build file, [**CMakeLists.txt**](https://github.com/ABRG-Models/morphologica/blob/main/standalone_examples/schnakenberg/CMakeLists.txt) assumes you did a
'morphologica install' into **/usr/local**, whereas
[**CMakeLists_intree.txt**](https://github.com/ABRG-Models/morphologica/blob/main/standalone_examples/schnakenberg/CMakeLists_intree.txt) will (if renamed to CMakeLists.txt) build the code assuming that you
placed a copy of the morphologica source  tree *inside* *standalone_examples/schnakenberg*.

### Legacy code and tests

To use morphologica, you don't need to 'build the library', because it is header-only. However, it has some test and example programs which
can be built with cmake. The process to build and install morphologica's
test/example programs is given in [README.install.linux.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.linux.md) for **GNU/Linux**,
[README.install.mac.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.mac.md) for **Apple Mac** and [README.install.windows.md](https://github.com/ABRG-Models/morphologica/blob/main/README.install.windows.md) for **Windows/Visual Studio**.

## Credits

Authorship of morphologica code is given in each file. Copyright in
the software is owned by the authors. morphologica is distributed
under the terms of the GNU General Public License, version 3 (see
[LICENSE.txt](https://github.com/ABRG-Models/morphologica/blob/main/LICENSE.txt)).
