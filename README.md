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

## Installation

Morphologica can be used in your code tree without installation. Just
include the path to morphologica when compiling, so that 
```c++
#include <morph/Header.h>
```
will work. morphologica has a
library of legacy code and some test programs which are built with cmake
and require OpenCV, OpenGL, armadillo, GLFW and HDF5 libraries as 
dependencies. The process to build and install morphologica is given in 
README.install.linux.md for GNU/Linux and README.install.mac.md for Apple Mac.

## Credits

Authorship of morphologica code is given in each file. Copyright in
the software is owned by the authors. morphologica is distributed
under the terms of the GNU General Public License, version 3 (see
LICENSE.txt).
