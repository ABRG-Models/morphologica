# Coding with morphologica

These are some instructions to help a new morphologica user out. As a
header-only library, morphologica is not difficult to incorporate into
your own code. This document describes some of the main classes in
morphologica and also includes sections describing what libraries you
will need to link to (for example, if you use ```morph::HdfData``` you
will need the [HDF5](https://www.hdfgroup.org/solutions/hdf5) library).

First off is a Helloworld example.

## A simple example

Here's a "Helloworld" example that writes some text on a window.

```c++
#include <morph/Visual.h>
int main()
{
    morph::Visual v(600, 400, "Hello World!");
    v.addLabel ("Hello World!", {0,0,0});
    while (v.readyToFinish == false) {
        glfwWaitEventsTimeout (0.018);
        v.render();
    }
    return 0;
}
```

This makes use of ```morph::Visual``` and creates a window with the
title "Hello World!" within which is written "Hello World!". It's an
empty Visual scene with a text label and nothing else. However, try
pressing 'c' in the window, and you'll see the 3D coordinate system
arrows appear. Press 'x' to exit.

This program is in morphologica's examples, so you can
compile and run it with:

```bash
cd morphologica
mkdir build
cd build
cmake ..
make helloworld
./examples/helloworld
```

The easiest way to hack on a simple example is to copy one of the
example programs, and add a new entry to examples/CMakeLists.txt so
that your new example will compile.

I *can* compile the helloworld program with a single g++ call on my Ubuntu machine...
```bash
cd /home/seb/models/morphologica/examples
/usr/bin/g++ \
    -I/home/seb/models/morphologica \                                  # Include dirs
    -I/home/seb/models/morphologica/include \
    -isystem /usr/include/jsoncpp \
    -isystem /usr/include/freetype2  \
    -D__GLN__ -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES \                 # Important flags to set
    -DMORPH_FONTS_DIR="\"/home/seb/models/morphologica/fonts\"" \
    -Wall -g -Wfatal-errors -Wno-unused-result -Wno-unknown-pragmas \  # Almost all warnings
    -march=native -O3 -fopenmp -std=gnu++17 \                          # More compiler flags
    -o helloworld helloworld.cpp  \                                    # Input code file and name for output
    /usr/lib/x86_64-linux-gnu/libglfw.so.3.3 \                         # Links to shared libraries
    /usr/lib/x86_64-linux-gnu/libfreetype.so \
    /usr/lib/x86_64-linux-gnu/libGLX.so \
    /usr/lib/x86_64-linux-gnu/libOpenGL.so
```
...but as you can see, there are quite a few includes and links to
keep track of, and so I find it easier to use cmake! (It's always nice
to see the single compile command, though).

## OpenGL graphics with morph::Visual

### Introduction to morph::Visual

Let's get straight into using the modern OpenGL visualisation code
that's probably the reason you're reading this document. The class
that makes all this possible is
[morph::Visual](https://github.com/ABRG-Models/morphologica/blob/main/morph/Visual.h). morph::Visual
was designed to be a 'game engine for visualisation' that would
provide an environment into which you can place 'VisualModel
objects'. Each morph::Visual is related to one window within your
desktop environment and the VisualModels are the objects that appear
in the window (each object has to derive from
[morph::VisualModel](https://github.com/ABRG-Models/morphologica/blob/main/morph/VisualModel.h).

morph::Visual contains all the difficult-to-write OpenGL code. It
allows the user to pan and zoom the environment to view 3D
visualisations from any angle. It bundles fonts and text-handling code
to allow you to render anti-aliased text in your models. It provides
shaders that give you lighting and alpha-blend effects. It also
provides the facilities to save an image of the scene, or save the
graphical portions of the scene into a
[glTF](https://github.com/KhronosGroup/glTF) file (so that you could
open them in Blender). If you want to make a custom visualisation for
a simulation, the only job you have to do is to describe what shapes
will make up the graphical model. And because it's *modern* OpenGL,
the graphics rendering is really fast, so you can visualise your
simulations in realtime.

A basic program will create a ```morph::Visual``` instance, set some
of its parameters, add some VisualModels and then call the
```Visual::render()``` method each time the screen should be updated.

Here's an example of the code used to create a Visual instance.

```c++
// Create a 1024 pixel by 768 pixel morph::Visual window titled "Example"
morph::Visual v(1024, 768, "Example");
// Choose a black background
v.backgroundBlack();
// Switch on a mix of diffuse/ambient lighting
v.lightingEffects(true);
```

Now that the Visual instance exists, you can add VisualModels. You'll
see a lot of ```morph::Vector<float, 3>``` objects. These are very much like
(and in fact are derived from) ```std::array<>``` from the standard library.

```c++
// Each VisualModel is given an 'offset within the Visual
// environment'. Use this offset to control the relative locations of
// all your VisualModel objects.
morph::Vector<float, 3> offset = { 0.0, 0.0, 0.0 };

// Here, I'm using a very simple morph::TriangleVisual to draw a
// triangle on the screen. I have to specify three corners. These
// coordinates are in 'VisualModel space'
morph::Vector<float, 3> c1 = { 0, 0, 0 };
morph::Vector<float, 3> c2 = { 0.25, 0, 0 };
morph::Vector<float, 3> c3 = { 0.0, 0.3, 0 };

// The last piece of information that the TriangleVisual will
// require is a colour. This is an RGB triplet, so this triangle will
// be red.
morph::Vector<float, 3> colour1 = { 1.0, 0.0, 0.0 };

// Now create the TriangleVisual. You allocate memory for the
// model here; morph::Visual will be responsible for deallocating the
// memory, as long as you add the VisualModel-derived object to the Visual...
morph::TriangleVisual* tv = new morph::TriangleVisual (v.shaderprog, offset, c1, c2, c3, colour1)

// ...like this:
v.addVisualModel (tv);
```

Now your ```morph::Visual``` contains one ```VisualModel```, which has all
the information required to specify how it should look. To actually
render it on the screen, you call ```morph::Visual::render()```:

```c++
v.render();
```

This draws the scene once. In order to have a responsive scene that
you can drag and move with the mouse, you have to process window
events and update the scene with ```v.render()``` at a suitable frequency. That happens like this:

```c++
while (v.readyToFinish == false) {
    glfwWaitEventsTimeout (0.018);
    v.render();
}
```

```glfwWaitEventsTimeout (0.018);``` waits for mouse and keyboard events for
0.018 seconds, then ```v.render()``` is called. This continues for as long
as the window should remain open. When the user presses the 'x' key,
the attribute ```Visual::readyToFinish``` is set to true and this loop will
be exited.

The example code in the introduction is adapted from
[examples/tri.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/tri.cpp). The
program window looks like this:

![A triangle rendered in a window by tri.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/screenshots/tri.png?raw=true)

### A VisualModel example: morph::TriangleVisual

There are a number of ready-made VisualModel-derived classes that you
can use to create your visualisations. It is expected that you will use
existing VisualModels like
[GraphVisual](https://github.com/ABRG-Models/morphologica/blob/main/morph/GraphVisual.h)
and also create your own specialised classes that derive from
VisualModel
(see for example, [netvisual](https://github.com/ABRG-Models/morphologica/blob/main/examples/SimpsonGoodhill/netvisual.h)).

Let's look at a simple one first; the
[morph::TriangleVisual](https://github.com/ABRG-Models/morphologica/blob/main/morph/TriangleVisual.h) class that we just encountered:

```c++
class TriangleVisual : public VisualModel
{
public:
    TriangleVisual() { this->mv_offset = {0.0, 0.0, 0.0}; }

    //! Initialise with offset, three coordinates and a single colour.
    TriangleVisual(GLuint sp, const Vector<float, 3> _offset,
                   const Vector<float, 3> _coord1, const Vector<float, 3> _coord2, const Vector<float, 3> _coord3,
                   const std::array<float, 3> _col)
    {
        this->init (sp, _offset, _coord1, _coord2, _coord3, _col);
    }
...
```

```TriangleVisual``` derives from ```VisualModel``` and has
two constructors. The second constructor takes an argument ```GLuint
sp``` which is the 'shader program' id. You'll see this in all
VisualModels; this shader program (and a corresponding text shader
program id) is a handle that has to be passed down from the
morph::Visual (which manages the OpenGL context) to all VisualModels.

I've collected all the initialisation into a method called
```init()```. Although this isn't strictly necessary, it follows a
convention in other VisualModel-based classes in which I attempt to
minimise the number of arguments passed to the constructor.

```c++
void init (GLuint sp, const Vector<float, 3> _offset,
           const Vector<float, 3> _coord1, const Vector<float, 3> _coord2, const Vector<float, 3> _coord3,
           const std::array<float, 3> _col)
{
    // Keep a copy of the shader program handle/id - common to all VisualModels
    this->shaderprog = sp;
    // Keep a copy of the offset of this Visual model within the scene - common to all VisualModels
    this->mv_offset = _offset;
    // Set this offset into the VisualModel's viewmatrix - common to all VisualModels
    this->viewmatrix.translate (this->mv_offset);

    // Copy the three triangle vertex coordinates and the colour. This is TriangleVisual-specific code
    this->coord1 = _coord1;
    this->coord2 = _coord2;
    this->coord3 = _coord3;
    this->col = _col;

    // Initialize the vertices that will represent the object. All VisualModels have initializeVertices().
    this->initializeVertices();

    // postVertexInit() is VisualModel code that sets up the OpenGL buffers from the vertices that were just initialized.
    this->postVertexInit();
}
```

To make your own VisualModel-based classes, you'll write most of your
code for the ```initializeVertices``` method. Its task is to populate
four vectors: ```vertexPositions```, ```vertexNormals```,
```vertexColors``` and ```indices```. In OpenGL, graphical models are
built up by specifying a mesh of triangles. ```vertexPositions```
holds the vertices of all the triangles that form the model. In the
case of this TriangleVisual, there will be exactly 3 coordinates in
```vertexPositions```; each coordinate has 3 dimensions, and so
```vertexPositions``` (which is of type ```std::vector<float>```) will
contain 9 floats. ```vertexNormals``` holds the normal vectors for
each vertex in the model (these determine how light is scattered, for
one thing) and ```vertexColors``` contains an RGB colour value for
each vertex. In this class, all the 3 vertices have the same
colour. ```indices``` gives the sequence in which the vertices should
be turned into triangles. In general, there will likely be more
entries in ```indices``` than there are coordinates in
```vertexPositions``` because adjacent triangles in a model will share
vertex positions. However, in this simple VisualModel, there is only a
single triangle and so there are exactly three entries in
```indices```. By populating these four vectors, you define a model
which can be rendered in 3D by OpenGL.

At the end of ```init()```, ```postVertexInit``` is called. This does the magic of
'binding' the four vectors of indices, vertex positions, normals and
colours to OpenGL buffers. Now, when ```VisualModel::render()``` is called,
a call to ```glDrawElements``` triggers a render of the data in the
OpenGL buffers via the shader program. However, the idea behind
morph::Visual is that you shouldn't have to know about the OpenGL
internals. All you should have to learn about is how to populate
```vertexPositions``` and friends.

In TriangleVisual, initializeVertices looks like this:

```c++
//! Initialize vertex buffer objects and vertex array object.
void initializeVertices (void)
{
    // First empty out vertexPositions, etc:
    this->vertexPositions.clear();
    this->vertexNormals.clear();
    this->vertexColors.clear();
    this->indices.clear();

    // The indices index. Passed by reference to 'drawing' functions like computeTriangle here.
    VBOint idx = 0;
    // Draw a triangle. That's it.
    this->computeTriangle (idx, this->coord1, this->coord2, this->coord3, this->col);

    std::cout << "idx now has value: " << idx << std::endl;
    std::cout << "vertexPositions has size " <<  this->vertexPositions.size()<< std::endl;
}
```

The 'drawing primitive' here is ```computeTriangle()```, which takes as
arguments the 'indices index', the corners of the triangle and its
colour.

```c++
//! Compute a triangle from 3 arbitrary corners
void computeTriangle (VBOint& idx,
                      Vector<float> c1, Vector<float> c2, Vector<float> c3,
                      std::array<float, 3> colr)
{
    // v is the face normal
    Vector<float> u1 = c1-c2;
    Vector<float> u2 = c2-c3;
    Vector<float> v = u1.cross(u2);
    v.renormalize();
    // Push corner vertices
    this->vertex_push (c1, this->vertexPositions);
    this->vertex_push (c2, this->vertexPositions);
    this->vertex_push (c3, this->vertexPositions);
    // Push colours/normals
    for (size_t i = 0; i < 3; ++i) {
        this->vertex_push (colr, this->vertexColors);
        this->vertex_push (v, this->vertexNormals);
    }
    // Push indices
    this->indices.push_back (idx++);
    this->indices.push_back (idx++);
    this->indices.push_back (idx++);
}
```

Here, ```Vector<float>``` is a ```morph::Vector<float,3>``` and note that I can
do vector arithmetic with Vectors. The code adds each corner to
```vertexPositions```, using the convenience function ```VisualModel::vertex_push()```
(which adds all 3 elements of one coordinate to
vertexPositions/Colors/Normals in one call). It computes the
normal vector of the triangle's face, ```v```, and places this in vertexNormals
three times. It also places the single colour for the triangle into
```vertexColors``` three times. Lastly, it adds three indices to
```indices```. At the end of the function, the argument ```idx``` will
have been incremented by 3. This would be important were
```computeTriangle()``` to be called a second time to draw another triangle
within this VisualModel.

That completes the code to draw a triangle within the
framework of ```morph::Visual``` and ```morph::VisualModel```.

### Drawing primitives

To create a VisualModel, you have to decide what triangle mesh will
represent the model and then painstakingly work out the location of
each vertex. This is quite an involved job. However, if you want to
build a VisualModel containing simple objects such as spheres, discs,
tubes, cones and lines, then most of the hard work might have already
been done. VisualModel provides a number of 'drawing primitive'
functions that can be used in the ```initializeVertices``` function of
your newly derived class. The ```netvisual``` class from the
'SimpsonGoodhill' example does this:

```c++
template <typename Flt>
class NetVisual : public morph::VisualModel
{
public:
    void initializeVertices()
    {
        VBOint idx = 0;
        // Spheres at the net vertices
        for (unsigned int i = 0; i < this->locations->p.size(); ++i) {
            // This is VisualModel::computeSphere()
            this->computeSphere (idx, this->locations->p[i], this->locations->clr[i], this->radiusFixed, 14, 12);
        }
        // Connections
        for (auto c : this->locations->c) {
            morph::Vector<Flt, 3> c1 = this->locations->p[c[0]];
            morph::Vector<Flt, 3> c2 = this->locations->p[c[1]];
            std::array<float, 3> clr1 = this->locations->clr[c[0]];
            std::array<float, 3> clr2 = this->locations->clr[c[1]];
            // This is VisualModel::computeLine()
            this->computeLine (idx, c1, c2, this->uz, clr1, clr2, this->linewidth, this->linewidth/Flt{4});
        }
    }
...
```

This is a visualisation of a fishnet plot, which consists of a number
of spheres, joined up by lines to make a mesh. The two drawing
primitive functions ```computeSphere()``` and ```computeLine()``` are
found in ```morph::VisualModel```. Other drawing primitives include
```computeTube()```, ```computeFlatPoly()```, ```computeRing()``` and
```computeCone()```.

### Text

To describe how to add text to your VisualModel.

### Graphs

How to draw 2D graphs with the ```morph::GraphVisual``` class and 3D
graphs with the aid of ```morph::TriaxesVisual```.

### Dynamic graphs and VisualModels

How to go about updating a model to reflect new data computed by your
simulation.

## The morph::Config class

[morph::Config](https://github.com/ABRG-Models/morphologica/blob/main/morph/Config.h) reads and writes parameter configuration data in JSON format. JSON is
very easy to work with (certainly compared with XML files). The idea is that you will
write your parameters by hand (or with a script) into a JSON file,
then these are conveniently accessible in your program. Here's an
example from our Schnakenberg reaction-diffusion example,
schanakenberg.json:
```json
{
    "Section_Simulation" : "Simulation settings",
    "steps" : 125000,
    "logevery" : 5000,
    "logpath" : "logs/",
    "saveplots" : false,
    "dt" : 0.000005,

    "Section_Parameters" : "Schnakenberg model parameters",
    "D_A" : 0.5,
    "D_B" : 0.6,
    "k1"  : 3,
    "k2"  : 1,
    "k3"  : 2,
    "k4"  : 2
}
```
The code to read this is easy to use:
```c++
#include <morph/Config.h>
{
    morph::Config conf("./params.json");
    if (!conf.ready) { /* handle error */ }
    // The length of one timestep. If dt does not exist in the JSON
    // file, then the default value 0.00001 is written into the variable dt.
    const double dt = conf.getDouble ("dt", 0.00001);
    // Each type has a named method for access. Here's unsigned int:
    const unsigned int logevery = conf.getUInt ("logevery", 1000);
    // And a string:
    std::string logpath = conf.getString ("logpath", "fromfilename");
}
```
At the end of your program, it's a good idea to write out a copy of
the params.json file into your log directory, along with some
information about the simulation run:

```c++
{
    // Add information into the existing morph::Config object:
    conf.set ("float_width", (unsigned int)sizeof(FLT)); // FLT is a template param
    std::string tnow = morph::Tools::timeNow();
    conf.set ("sim_ran_at_time", tnow);
    if (argc > 0) { conf.set("argv0", argv[0]); }
    if (argc > 1) { conf.set("argv1", argv[1]); }
    conf.insertGitInfo ("sim/"); // checks 'sim' dir for any post-commit changes
    const std::string paramsCopy = logpath + "/params.json";
    conf.write (paramsCopy);
    if (conf.ready == false) { /* handle error */ }
}
```

### Config overrides and batch simulations

When developing a numerical model, it is often useful to sweep through many different values of one parameter, to find out how it affects the model's behaviour. Often, this means writing a script to launch a number of simulations, each with slightly different parameter settings. In this case, it would be tiresome to create a new JSON file for each individual simulation run; it is much easier to write a single JSON file with all the common parameters, and then be able to override one parameter with a new value for each run. Here's how you can do this:

When you have created your ```morph::Config``` object, you pass your main function's argc and
argv to ```morph::Config::process_args(argc, argv)```. This records any
'overrides' that you've placed into the command line of your
program. 

```c++
#include <morph/Config.h>

int main (int argc, char** argv)
{
    morph::Config conf("./params.json");
    if (!conf.ready) { /* handle error */ }
    conf.process_args (argc, argv); // This line is all you have to add to your setup!

    // Now, when you call conf.getDouble for the parameter 'A', then you'll 
    // get the one from JSON, unless the command line has an override for A.
    const double dt = conf.getDouble ("A", 1);
}
```

You do have to use a fixed format for the overrides on your command line. So, in an
example program where you want to update the parameter 'A' in your
config, you'd run like this:
```
myprog -co:A=4.5
```
The ```'-co:'``` is the token that ```Config::process_args``` uses to recognise
that you're overriding the value of A that is given in your existing
Config ('co' stands for Config Override).

You can then run several programs:
```
myprog -co:A=4.6
myprog -co:A=4.7
```
and so on.

You can also use multiple instances, and specify texts and booleans,
like this:
```
myprog -co:A=4 -co:B=5 -co:C=text -co:D=false
```
Finally, you can add any of your other usual command line args, as long as they don't match ```'-co:'```.
```
myprog ./path/to/myconf.json -p -s -co:A=4 -co:z=true
```

## The morph::HdfData class

[morph::HdfData](https://github.com/ABRG-Models/morphologica/blob/main/morph/HdfData.h)
is a C++ wrapper around the HDF5 C API. There are other wrappers
available, but I wanted a relatively simple one, and this is it.

### Writing with HdfData

With ```HdfData```, you can write out individual numbers and containers of
numbers into the HDF5 file. Mostly, that will be arrays of numbers,
such as ```std::vector<double>```. To save some arrays like this into
a file is very simple:

```c++
#include <morph/HdfData.h>
#include <vector>
int main()
{
    // Two vectors of doubles:
    std::vector<double> vd1 = { 10.0, 12.0, 13.0, 14.0 };
    std::vector<double> vd2 = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    {
        morph::HdfData data("test.h5", morph::FileAccess::TruncateWrite);
        data.add_contained_vals ("/vd1", vd1);
        data.add_contained_vals ("/vd2", vd2);
    } // data closes when out of scope
    return 0;
}
```

You just create an object of type ```morph::HdfData``` and say whether you
want write to the file (as I've done here with
the ```morph::FileAccess::TruncateWrite``` argument) or whether you want to
read it (```morph::FileAccess::ReadOnly```).

Note how I've used the scope identifier brackets ```{``` and ```}``` to place the ```HdfData```
object in its own scope. When the HdfData object goes out of scope,
the deconstructor is called and the file is closed. After than you can
safely open the file in another program.

Writing data is just a couple of calls of ```HdfData::add_contained_vals```.

You can use ```HdfData::add_contained_vals``` to write quite a few
combinations of container and contained type. Here are some that will
work just fine:

```c++
std::vector<float> d1;
std::list<int> d2;
std::deque<unsigned int> d3;
std::vector<std::array<float, 3>> d4;
morph::vVector<morph::Vector<float>> d5;
```

If you want to write just a single value into your h5 file, then the
function to use is called ```HdfData::add_val```:

```c++
double d = 5;
{
    morph::HdfData data("test.h5", morph::FileAccess::TruncateWrite);
    data.add_val ("/a_number", d);
}
```

### Reading with HdfData

Reading from your HDF5 file is mostly as easy as writing to it. If you
write with ```add_contained_vals```, then the corresonding read function is
```read_contained_vals```. Here's an example:

```c++
std::vector<double> vdread;
{
    morph::HdfData data("test.h5", morph::FileAccess::ReadOnly);
    data.read_contained_vals ("/vd1", vdread);
}
```

To read a single value that you saved with ```HdfData::add_val```, you use ```HdfData::read_val```:

```c++
double d = 0.0;
{
    morph::HdfData data("test.h5", morph::FileAccess::ReadOnly);
    data.read_val ("/a_number", d);
}
```

### Appending with HdfData

So you created an HDF5 file, filled it with data and closed it. Now you want to add some more data to the file.
Sounds like it should be easy, but it turns out that to do this requires quite a lot more effort working with the HDF5 internals.
You have to tell the API ahead of time that your data arrays might get larger and that there might be new arrays added in the future.
This is all such a headache, that I've not implemented it in HdfData.
Don't let the existence of the flag ```morph::FileAccess::ReadWrite``` mislead you;
I haven't implemented appending so if you need it, you'll either have to access the HDF5 API directly, or use one of the more comprehensive C++ wrappers.

### Reading and writing strings with HdfData

One other pair of HdfData functions that you might find useful are ```HdfData::add_string``` and ```HdfData::read_string```:
```c++
{
    morph::HdfData data("test.h5", morph::FileAccess::TruncateWrite);
    data.add_string ("/a_string", "Hello world");
}
std::string str("");
{
    morph::HdfData data("test.h5", morph::FileAccess::ReadOnly);
    data.read_string ("/a_string", str);
}
```

For more example code, you can look at examples/hdfdata.cpp and
tests/testhdfdata1.cpp (and testhdfdata2.cpp and testhdfdata3.cpp)

## morph::HexGrid

A class to manage simulations carried out on hexagonal grids with
arbitrary boundaries.

## morph::Scale

A class for data scaling, with autoscaling features.

## morph::Vector

This is an extension of std::array to make a class for mathematical vector
manipulation in N dimensions.

While you *can* just store your vectors in std::array, here are 15
things that you can do with morph::Vector<> (a mathematical vector
class) that require much more code with plain std::array:

```c++
#include <morph/Vector.h>
using morph::Vector;
{
    // 1 Access by named components:
    Vector<float, 3> v = {1,2,3};
    cout << "x: " << v.x() << endl;
    cout << "y: " << v.y() << endl;
    cout << "z: " << v.z() << endl;
    // 2 Send to cout:
    cout << "This vector is: " << v << endl;
    // 3 Renormalize the vector to length 1:
    v.renormalize();
    // 4 Check if it's a unit vector:
    cout << "is it unit? " << v.checkunit() << endl;
    // 5 Randomize the vector's components:
    v.randomize();
    // 6 Get its vector length:
    cout << "Length: " << v.length() << endl;
    // 7 Negate the vector
    Vector<int, 2> vi = {1,2,3};
    Vector<int, 2> vi3 = -vi;
    // 8 Compute the cross product (3D only)
    Vector<double, 3> a = {1,0,0};
    Vector<double, 3> b = {0,1,0};
    Vector<double, 3> c = a * b;
    cout << a << "*" << b << "=" << c << endl;
    // 9 Compute the dot product
    Vector<int, 2> vv1 = {1,1};
    Vector<int, 2> vv2 = {2,2};
    int dp = vv1.dot (vv2);
    // 10-13 Scalar multiply, divide, add, subtract
    vv2 *= 2UL;
    vv2 = vv2 / 5;
    Vector<int, 2> vv;
    vv = vv1 + 7;
    vv = vv1 - 9;
    // 14 Vector addition
    Vector<double, 3> e = a+b;
    cout << "a + b = " << e << endl;
    // 15 Vector subtraction
    Vector<double, 3> f = a-b;
    cout << "a - b = " << f << endl;
}
```

There's *even more* that you can do with a morph::Vector, take a look in the [header](https://github.com/ABRG-Models/morphologica/blob/main/morph/Vector.h).

## BezCurve, BezCurvePath and BezCoord

Classes to create Bezier curves.

## Linking a morphologica program

All of the morphologica classes are *header-only*, which means there is no 'libmorphologica' to link to your program. However, some of the classes need to link to 3rd party dependencies. Some of the main dependencies are:

* morph::HdfData: Link to ```libhdf5```. If you want to save/load OpenCV data structures then you need to link to OpenCV, too (and ```#define BUILD_HDFDATA_WITH_OPENCV``` before ```#include <morph/HdfData.h>```).
* morph::Visual: This uses 3D graphics, so it needs to link to OpenGL, GLFW3 and Freetype.
* morph::BezCurve: Link to ```libarmadillo```. Used for matrix algebra.
* morph::HexGrid and morph::CartGrid: These use BezCurves, so need ```libarmadillo```.

Some morphologica classes use no third party code. ```morph::Vector```, for example is very much standalone.

This is the only real headache of working with morphologica: working out the right compiler line to call to compile a morphologica program.

I use cmake to coordinate includes and links. cmake examples are given in the [top level readme](https://github.com/ABRG-Models/morphologica/blob/main/README.md) to
show how to set up the includes, compiler flags and links using that system.

## CMakeLists.txt for your project

To include a description of writing your own CMakeLists.txt file.
