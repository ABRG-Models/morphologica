# morphologica

Library code used in models developed by Stuart P. Wilson, Seb James
and co-workers in the Wilson Lab.

This header-only c++ code installs headers which
contain **simulation support facilities** for our simulations of
dynamical systems.

It helps with:

* **Configuration**: morphologica allows you to easily set up a simulation
  parameter configuration system, using the JSON reading and writing
  abilities of **morph::Config**.

* **Saving data from your simulation**. morphologica provides a set of
  easy-to-use convenience wrappers (**morph::HdfData**) around the HDF5 C
  API. By saving data in a standard format, it is easy to access
  simulation data in python, MATLAB or Octave for analysis and graphing.

* **Visualizing your model while it runs**. A modern OpenGL visualization
  scheme called **morph::Visual** provides the ability to visualise hex
  grids, surfaces, scatter plots and quiver plots with minimal
  processing overhead.

It keeps *out of the way* of what kind of simulation you write. Our
programs typically start with some kind of preamble, in which we use
morph::Config to load up a JSON parameter file defining the values of
the parameters for the simulation run. We might also use
morph::HdfData to retrieve some data (e.g. the state) from an earlier
simulation and then set up a morph::Visual object for the
visualization. We then might call a function, or create a class object
which defines the simulation. *This may or may not access features
from libmorphologica*.

As the simulation progresses, we update the data in the morph::Visual
scene; save images from the scene for movie making and record data as
often as we want it using morph::HdfData. At the end of the program,
as well as saving any final data, we use morph::Config to save out a
'version of record' of the parameters that were used, along with git
information which morph::Config can extract so that we could find the
exact version of the simulation for future reproducion of the result.

Although it need not be incorporated into your actual simulation,
morphologica does also provide classes that you might find
useful. Examples include:

* **morph::HexGrid**: a class for running simulations on hexagonal
grids (it manages all the neighbour relationships between hexes and
allows you to specific various boundary shapes for your domain)

* **morph::Vector** and **morph::vVector**: Cool vector classes.

* **morph::MathAlgo** a class containing mathematical algorithms.

* **morph::BezCurve** and friends: classes for working with Bezier
    curves.

* **morph::Winder** A class to compute the winding number of a path.

* **morph::Scale** A class for simple scaling/transformation of numbers.

* **morph::NM_Simplex** An optimization algorithm.

* **morph::RandUniform** and friends. Nice wrapper classes around
    c++'s high quality random number generation code.

* **morph::ReadCurves** Code to read SVG drawings to turn Bezier-curve
    based lines into paths containing evenly spaced coordinates.

morphologica is a way of storing our 'group knowledge' for posterity!

Some existing projects which use morphologica are:
* **BarrelEmerge** A reaction-diffusion style model: https://github.com/ABRG-Models/BarrelEmerge
* **RetinoTectal** Also reaction-diffusion: https://github.com/sebjameswml/RetinoTectal
* **ArtificialGeneNets** Neural networks: https://github.com/stuartwilson/ArtificialGeneNets

## Code documentation

morphologica code is enclosed in the **morph** namespace. You can see
the doxygen-generated code documentation at
https://codedocs.xyz/ABRG-Models/morphologica/

For full, compilable examples of the code, see the examples/ subdirectory.

See README.coding.md for a quick-start guide to the main classes.

## Building code against morphologica

To build, you need to tell your build process: **1**) Where the
morphologica headers are to be found. **2**) Where the fonts that
morphologica will compile into your binaries (if you're using
morph::Visual) are located. **3**) which libraries to link to.

You can either build with morphologica headers (and fonts) installed
in your chosen location (/usr/local by default) *or* you can just
clone a copy of morphologica source into your own source tree.

### Option 1: With morphologica installed

If you would like to work with morphologica headers installed in
/usr/local, then this is what you add to your client code's
CMakeLists.txt (assuming you're using cmake to build your own code).

#### 1) Includes

```cmake
# Find the libraries which will be needed
find_package(jsoncpp REQUIRED)
find_package(HDF5 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(OpenCV REQUIRED)
find_package(LAPACK REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define collections of includes for the dependencies
set(MORPH_INC_CORE ${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MORPH_INC_CORE} ${MORPH_INC_GL})

# Where did you install morphologica?
set(MORPH_INCLUDE_PATH /usr/local)
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include)
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include/morph)
```

#### 2) Ability for your compiler to copy in the fonts

Set a definition to say where there are some truetype fonts that will
be compiled into your binaries.

```cmake
# Tell the program where the morph fonts are. Again, assuming you installed morphologica in /usr/local:
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"/usr/local/share/fonts\\\"\"")
```

#### 3) Links

```cmake
set(MORPH_LIBS_CORE ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} ${LAPACK_LIBRARIES} jsoncpp_lib)
set(MORPH_LIBS_GL ${OpenCV_LIBS} OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(myprogtarget ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
```

### Option 2: With morphologica 'in-tree'

Seb prefers this option: 'clone and go'. Just git clone morphologica
into your source tree (or somewhere else) and then make sure your
compiler can find the includes, fonts and links.

#### 1) Includes
```cmake
# Find the libraries which will be needed
find_package(jsoncpp REQUIRED)
find_package(HDF5 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(OpenCV REQUIRED)
find_package(LAPACK REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)

# Define collections of includes for the dependencies
set(MORPH_INC_CORE ${ARMADILLO_INCLUDE_DIR} ${ARMADILLO_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
set(MORPH_INC_GL ${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLFW3_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MORPH_INC_CORE} ${MORPH_INC_GL})

# Where did you install morphologica? Here, I assume you git cloned it
into the root of your own project source tree.
set(MORPH_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/morphologica" CACHE PATH "The path to the morphologica headers (e.g. /usr/local/include or \$HOME/usr/include)")

include_directories(BEFORE ${MORPH_INCLUDE_PATH})
include_directories(BEFORE ${MORPH_INCLUDE_PATH}/include)

```

#### 2) Ability for your compiler to copy in the fonts

Set a definition to say where there are some truetype fonts that will
be compiled into your binaries.

```cmake
# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMORPH_FONTS_DIR=\"\\\"${PROJECT_SOURCE_DIR}/morphologica/fonts\\\"\"")
```

#### 3) Dependency library linking

```cmake
set(MORPH_LIBS_CORE ${ARMADILLO_LIBRARY} ${ARMADILLO_LIBRARIES} ${HDF5_C_LIBRARIES} ${LAPACK_LIBRARIES} jsoncpp_lib)
set(MORPH_LIBS_GL ${OpenCV_LIBS} OpenGL::GL Freetype::Freetype glfw)
target_link_libraries(myprogtarget ${MORPH_LIBS_CORE} ${MORPH_LIBS_GL})
```

morphologica has a library of legacy code and some test programs which
are built with cmake. The process to build and install morphologica's
test programs is given in README.install.linux.md for GNU/Linux and
README.install.mac.md for Apple Mac.

## Credits

Authorship of morphologica code is given in each file. Copyright in
the software is owned by the authors. morphologica is distributed
under the terms of the GNU General Public License, version 3 (see
LICENSE.txt).
