# Coding with morphologica

These are some instructions to help a new morphologica user out.

## Build process

All of the morphologica classes are *header-only*, which means there is no 'libmorphologica' to link to your program. However, some of the classes need to link to 3rd party dependencies. Some of the main dependencies are:

* morph::Config: Link to ```libjsoncpp```
* morph::HdfData: Link to ```libhdf5```. If you use OpenCV methods in morph::HdfDataCV, you may need to link to OpenCV, too
* morph::Visual: This uses 3D graphics, so it needs to link to OpenGL, GLFW3, Freetype and libjsoncpp.
* morph::BezCurve: Link to ```libarmadillo```. Used for matrix algebra.
* morph::HexGrid and morph::CartGrid: These use BezCurves, so need ```libarmadillo```.

Some morphologica classes use no third party code. ```morph::Vector```, for example is very much standalone.

This is the only real headache of working with morphologica: working out the right compiler line to call to compile a morphologica program.

I use cmake to coordinate includes and links. cmake examples are given in the [top level readme](https://github.com/ABRG-Models/morphologica/blob/main/README.md) to
show how to set up the includes, compiler flags and links using that system.

## A simple example

Here's a "Helloworld" example.

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
cd /home/seb/morphologica/examples
/usr/bin/g++ \
-I/home/seb/morphologica \
-I/opt/graphics/OpenGL/include \
-I/home/seb/morphologica/include \
-isystem /usr/include/freetype2 \
-D__GLN__ -Wall -g -Wfatal-errors -Wno-unused-result \
-Wno-unknown-pragmas -march=native -O3 -fopenmp \
-DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES \
-DMORPH_FONTS_DIR="\"/home/seb/morphologica/fonts\"" \
-std=gnu++17 \
-o helloworld helloworld.cpp \
-lopenblas -lpthread -lm \
/usr/local/lib/libglfw3.a \
/usr/lib/x86_64-linux-gnu/libfreetype.so \
/usr/local/lib/libjsoncpp.a \
/usr/lib/x86_64-linux-gnu/libGLX.so \
/usr/lib/x86_64-linux-gnu/libOpenGL.so -lrt -ldl -lX11
```
...but as you can see, there are quite a few includes and links to keep track of, and so I find it easier to use cmake! (It's always nice to see the single compile command, though).

## The morph::Config class

[morph::Config](https://github.com/ABRG-Models/morphologica/blob/main/morph/Config.h) reads and writes parameter configuration data in JSON format. JSON is
*so much easier* to work with than XML! The idea is that you will
write out your parameteres by hand (or with a script) in a JSON file,
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

## The morph::Visual class for Visualising your simulations

Modern OpenGL visualisation code.

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

## CMakeLists.txt for your project

To include a description of writing your own CMakeLists.txt file.
