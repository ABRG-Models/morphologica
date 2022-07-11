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

An example demonstrating the use of morph::HexGrid, its
HexGrid::convolve function, the morph::Random class and a
visualization of the input, the convolution kernel and the resulting
output.

![Screenshot of the convolve program](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/convolve.png?raw=true)

### logisticmap.cpp

Computes the logistic map and displays with a morph::GraphVisual, using diamond shaped markers.

![Screenshot of the logisticmap program](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/logisticmap.png?raw=true)

### rosenbrock.cpp

This program find the minimum of the Rosenbrock banana function using
the Nelder-Mead simplex optimization method (coded as the class
morph::NMSimplex). The walk of the simplex down the function surface
is animated. Note that the scaling of the colour map is set so that
only the lowest part of the surface is resolved in the green to blue
part of the map (which is Inferno)

![Screenshot of Rosenbrock banana function with Nelder-Mead triangle](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/rosenbrock.png?raw=true)

### Ermentrout2009

Contains an implementation of a Keller-Segel reaction diffusion system.

**From the base of morphologica**, run like this (the program needs to access the file ./boundaries/whiskerbarrels.svg):

```bash
/build/examples/Ermentrout2009/erm ./examples/Ermentrout2009/configs/erm.json
```

![Screenshot of the erm.cpp program, showing a bullseye mode](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/erm.png?raw=true)

### LotkaVolterra

Contains an implementation of the Lotka-Volterra population model cast as a reaction diffusion system.

![Shows two surfaces for two population variables, u and v](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/lotkavolterra.png?raw=true)

### SimpsonGoodhill

Contains an implementation of the model described in the paper 'A
simple model can unify a broad range of phenomena in retinotectal map
development', Biological Cybernetics, 2011, by Hugh Simpson and Geoffrey Goodhill. This one is notable because it is an **agent based model**, rather than a reaction diffusion model. Proof that morphologica is useful for many different types of model! The image shows results for the wildtype model, Figs 2B to 2D in the paper, with parameters exactly as given in the paper.

**From the base of morphologica**, run like this:

```bash
/build/examples/SimpsonGoodhill/sg ./examples/SimpsonGoodhill/sg.json
```
![Screenshot of the sg.cpp program, showing the wildtype model (cf. Figs 2B-2D)](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/sg.png?raw=true)

## morph::Visual examples

These simple examples showcase the features in morphologica's
morph::Visual code. They're a useful place to see what the code can do
for data visualization.

### visual.cpp

An example morph::Visual program which shows a morph::HexGrid and some
text labels.

![Screenshot of a morph::Visual scene](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/visual.png?raw=true)

### fps.cpp

A dynamic HexGrid program showing an animated surface

![Screenshot of the animated hexgrid program, fps.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/fps.png?raw=true)

### hexgrid.cpp

Example HexGridVisual

![Screenshot of hexgrid.cpp showing a sinusoidal landscape in the jet colour map](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/hexgrid.png?raw=true)

### hexgrid_image

Creates a HexGrid, then uses `HexGrid::resampleImage` to load a picture.

![Screenshot of hexgrid_image.cpp showing a resampled image of a touring bicycle](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/hexgrid_image.png?raw=true)

### cartgrid.cpp

Example Cartesian grid (CartGridVisual)

![Screenshot of cartgrid.cpp showing a sinusoidal landscape in the jet colour map](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/cartgrid.png?raw=true)

### quiver.cpp

An example quiver plot using morph::QuiverVisual.

![Screenshot of a 3D quiver plot](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/quiver.png?raw=true)

### scatter.cpp

An example three dimensional scatter plot of spheres using
morph::ScatterVisual. Note that in this example, the coordinate arrows
are set within the scene (and so move with the model).

![Screenshot of a 3D scatter plot](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/scatter.png?raw=true)

### graph1.cpp to graph4.cpp

Various examples of the use of morph::GraphVisual.

![Screenshot of graph3.cpp, showing some example graphs](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/graph3.png?raw=true)

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
