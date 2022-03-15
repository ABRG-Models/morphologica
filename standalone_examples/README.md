# Morphologica examples

This directory provides some example models demonstrating the use of
Morphologica. They build as if they were external code - so they don't
build as part of the morphologica build process; instead you build and
install the morphologica library, then cd into one of the examples and
start a fresh cmake build process there. These provide templates for
developing your own programs using morphologica.

Morphologica intends to provide you with the scaffolding you need to
write a C++ simulation. It gives you a way to write and load
configurations, and, importantly, save the config out alongside your
logs with runtime parameters (such as the date and time, the command
line args used and even the git commit version of your code). It
provides a nice, simple wrapper around the HDF5 library so that you
can save your simulation data into files which can be opened by many
different mathematical packages, including Python, MATLAB and GNU
Octave. It provides you with the HexGrid class, which makes it easier
to run simulations on hexagonal grids. You can apply arbitrary
boundary shapes too! Finally, it has a set of classes which implement
a modern OpenGL visualization scheme which is efficient enough to run
alongside your simulation.

## schnakenberg

This directory contains a complete example. After you have compiled and
installed morphologica, you should be able to change directory into
schnakenberg, create a build directory and carry out a cmake build

```bash
cd morphologica/standalone_examples/schakenberg
mkdir build
cd build
cmake ..
make
cd ..
```

This program demonstrates the creation of a reaction diffusion system
using the morph::RD_Base class. It is a good demonstration of the main
classes that Morphologica makes available. It demonstrates:

* The use of the morph::Config JSON configuration scheme. This makes
  it easy to bring parameters into your simulation.
* The morph::RD_Base class (which makes use of morph::Hexgrid), and
  how to specialise it into the RD_Schnakenberg class.
* The morph::HdfData scheme for saving your simulation data into HDF5 files
* morph::Visual for OpenGL visualisation of the hex grid while the
  simulation runs, including the use of morph::ColourMap.
* The morph::Tools class, which contains a collection of static
  utility functions.

Run it as follows (I'm assuming you already built morphologica and then
ran through the build instructions above):

```bash
./standalone_examples/build/schnakenberg ./examples/schnakenberg.json
```

You can interact with the window using your mouse. For some help
(which will appear on stdout) press 'h' (with the graphical window
focussed).

## neuralnet

This implements a feedforward neural network which can be trained to
classify the MNIST database of handwritten numerals. It demonstrates
the use of morph::RandUniform<> and morph::vVector<>. It shows how to
set up a CMakeLists.txt file to use morphologica. Because it doesn't
use any OpenMP pragmas for parallel execution, it shows how to write
the CMakeLists.txt *without* OpenMP.

```bash
cd morphologica/standalone_examples/neuralnet
mkdir build
cd build
cmake ..
make
cd ..
```

Then to run the example:

```bash
./build/ff_mnist # see also ff_small and ff_debug for debugging
```

## recurrentnet

A recurrent neural network example. Demonstrates the use of code in
the morph::recurrentnet namespace in RecurrentNetwork.h and
RecurrentNetworkModel.h.

```bash
cd morphologica/standalone_examples/recurrentnet
mkdir build
cd build
cmake ..
make
cd ..
```

For instructions on running the example, see the README.md file in
recurrentnet.


## Elman network

Elman 1990, Finding Structure in Time. An implementation of this
recurrent network. See README in the elmannet directory.
