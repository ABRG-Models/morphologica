# Example programs

This folder contains a set of example morphologica programs to help
new users to get started using the library.

These examples will build alongside the unit tests when you do a
morphologica build like this:

```bash
# Clone morphologica if you didn't already
git clone git@github.com/ABRG-Models/morphologica
cd morphologica
mkdir build
cd build
cmake ..
make
cd ..
```
You'll find the example program binaries in `build/examples`.

The examples are almost all built on the morph::Visual environment,
which means you can interact with the mouse. Right-button down allows
you to drag, Left-button down allows you to rotate. Press 't' to
change the axis of rotations. Press 'h' and have a look at stdout to
see some other key presses. 'x' exits. 'a' Resets the view.

## Computational and scientific model examples

These are examples of models that we've re-implemented from the
literature. These examples make use of most of the basic facilities in
morphologic; morph::Config, morph::HdfData and morph::Visual.

### convolve.cpp

![Screenshot of the convolve program](https://github.com/ABRG-Models/morphologica/blob/main/convolve.png?raw=true)

An example demonstrating the use of morph::HexGrid, its
HexGrid::convolve function, the morph::Random class and a
visualization of the input, the convolution kernel and the resulting
output.

### logisticmap.cpp

Computes the logistic map and displays with a morph::GraphVisual.

### Ermentrout2009

Contains an implementation of a Keller-Segel reaction diffusion system.

## morph::Visual examples

These simple examples showcase the features in morphologica's
morph::Visual code. They're a useful place to see what the code can do
for data visualization.

### visual.cpp

An example morph::Visual program which shows a morph::HexGrid and some
text labels.

### fps.cpp

A dynamic HexGrid program showing an animated surface

### hexgrid.cpp

Example HexGridVisual

### cartgrid.cpp

Example Cartesian grid (CartGridVisual)

### quiver.cpp

An example quiver plot using morph::QuiverVisual.

### scatter.cpp

An example three dimensional scatter plot of spheres using morph::ScatterVisual.

### graph1.cpp to graph4.cpp

Various examples of the use of morph::GraphVisual.

### twowindows.cpp

An example to show how to create two morph::Visuals, and hence two
windows, in your program.

### rods.cpp

An example of the very simple VisualModel, morph::RodVisual, which
simply draws a polygonal rod. You can specify how many sides, so this
can be used to draw rods of square section, or rods which appear to be
cylindrical.

### quads.cpp

An example of morph::QuadsVisual.

### pointrows.cpp

An example of morph::PointRowsVisual, which is used to render a
surface made of adjacent rows of points.

This program is also conditially compiled into the exectuable
pointrows_mesh, which renders the same points as a ball-and-stick
mesh.
