# Morphologica examples

This directory provides some example models demonstrating the use of
Morphologica. They build alongside the rest of the library, though
they are not installed with a 'make install'.

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

## schnakenberg.cpp

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

Run it as follows (I'm assuming you already built morphologica):

```bash
cd morphologica
./build/examples/schnakenberg ./examples/schnakenberg.json
```

You can interact with the window using your mouse. For some help
(which will appear on stdout) press 'h' (with the graphical window
focussed).
