# morphologica

![A banner image of a hexgrid surface plot](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/banner.png?raw=true)

**Header-only library code to visualize C++ numerical simulations using fast, modern OpenGL.**

You'll find all in the **code** in the [**morph**](https://github.com/ABRG-Models/morphologica/tree/main/morph) directory. For [**example code**](https://github.com/ABRG-Models/morphologica/tree/main/examples) and [**screenshots**](https://github.com/ABRG-Models/morphologica/tree/main/examples), see [this page](https://github.com/ABRG-Models/morphologica/tree/main/examples).

morphologica has some **demo/tutorial** content on YouTube: https://www.youtube.com/playlist?list=PLwiQ_IuTOr_Us9_tBde96VLYQlRWOYeAP

## Quick Start

This quick start shows dependency installation for Linux, because on this platform, it's a single call to apt (or your favourite package manager). If you're using a Mac, see [README.build.mac](https://github.com/ABRG-Models/morphologica/tree/main/README.build.mac.md) for help getting dependencies in place. It's [README.build.windows](https://github.com/ABRG-Models/morphologica/tree/main/README.build.windows.md) for Windows users.

```bash
# Install dependencies for building graph1.cpp (assuming Debian-like OS)
sudo apt install build-essential cmake git wget  \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev

git clone git@github.com:ABRG-Models/morphologica   # Get your copy of the morphologica code
cd morphologica
mkdir build         # Create a build directory
cd build
cmake ..            # Call cmake to generate the makefiles
make graph1         # Compile a single one of the examples. Add VERBOSE=1 to see the compiler commands.
./examples/graph1   # Run the program. You should see a graph of a cubic function.
# After closing the graph1 program, open its source code and modify something (see examples/graph2.cpp for ideas)
gedit ../examples/graph1.cpp
```
The program graph1.cpp is:
```c++
// Visualize a graph. Minimal example showing how a default graph appears
#include <morph/Visual.h>
#include <morph/GraphVisual.h>
#include <morph/vVector.h>

int main()
{
    // Set up a morph::Visual 'scene environment'.
    morph::Visual v(1024, 768, "Made with morph::GraphVisual");
    // Create a new GraphVisual with offset within the scene of 0,0,0
    auto gv = new morph::GraphVisual<double> (v.shaderprog, v.tshaderprog, {0,0,0});
    // Data for the x axis. A vVector is like std::vector, but with built-in maths methods
    morph::vVector<double> x;
    // This works like numpy's linspace() (the 3 args are "start", "end" and "num"):
    x.linspace (-0.5, 0.8, 14);
    // Set a graph up of y = x^3
    gv->setdata (x, x.pow(3));
    // finalize() makes the GraphVisual compute the vertices of the OpenGL model
    gv->finalize();
    // Add the GraphVisual OpenGL model to the Visual scene
    v.addVisualModel (gv);
    // Render the scene on the screen until user quits with 'x'
    v.keepOpen();
    // When v goes out of scope, gv will be deallocated
    return 0;
}
```
The program generates a clean looking graph...

![Screenshot of graph1.cpp output showing a cubic function](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/graph1.png?raw=true)

...and the code compares favourably (in terms of amount of boilerplate code) with the equivalent Python, graph1.py:
```Python
# Visualize the graph from graph1.cpp in Python
import matplotlib.pyplot as plt
import numpy as np

# Create some data for the x axis
x = np.linspace(-0.5, 0.8, 14)
# Set a graph up of y = x^3
plt.plot(x, np.power(x,3), '-o')
# Add labels
plt.title('Made with Python/numpy/matplotlib')
plt.xlabel('x')
plt.ylabel('y')
# Render the graph on the screen until user quits with 'q'
plt.show()
```
See the [coding README](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md) for a description of some of the main classes that morphologica provides.

## What is morphologica?

This header-only C++ code provides **simulation support facilities** for simulations of dynamical systems, agent-based models or, in fact, any program that needs dynamic, runtime visualization.

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

See [README.coding.md](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md) for a guide to the main classes.

morphologica code is enclosed in the **morph** namespace. If [README.coding.md](https://github.com/ABRG-Models/morphologica/blob/main/README.coding.md) doesn't coverit, then the header files (They're all in [morph/](https://github.com/ABRG-Models/morphologica/tree/main/morph)) contain code documentation.

You can find example programs which are compiled when you do the standard
cmake-driven build of morphologica in both the [tests/](https://github.com/ABRG-Models/morphologica/tree/main/tests) subdirectory
and the [examples/](https://github.com/ABRG-Models/morphologica/tree/main/examples) subdirectory. The readme in examples/ has some nice
screenshots.

For full, compilable, standalone examples of the code, see the
[standalone_examples/](https://github.com/ABRG-Models/morphologica/tree/main/standalone_examples) subdirectory. You can use these as templates for creating
your own projects that use morphologica library code.

For more info on how to set up CMake files to build a program using morphologica (and some hints as to what you'll need to do with an alternative directed build system), see [README.cmake.md](https://github.com/ABRG-Models/morphologica/blob/main/README.cmake.md).

## Credits

Authorship of morphologica code is given in each file. Copyright in
the software is owned by the authors.

morphologica is made possible by a number of third party projects whose source code is included in this repository. These include [nlohmann::json](https://github.com/nlohmann/json), [lodepng](https://github.com/lvandeve/lodepng), [rapidxml](http://rapidxml.sourceforge.net/) and [incbin](https://github.com/graphitemaster/incbin). Thanks to the authors of these projects!

morphologica is distributed under the terms of the Apache License, version 2 (see
[LICENSE.txt](https://github.com/ABRG-Models/morphologica/blob/main/LICENSE.txt)).
