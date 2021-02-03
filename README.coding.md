# Coding with morphologica

These are some instructions to help a new morphologica user out.

## Build process

See the end of the top-level readme for instructions on tailoring your
build process to work with morphologica - cmake examples are given to
show how to set up the includes, compiler flags and links that you'll
need to use the morphologica code.

## morph::Config

Reads and writes parameter configuration data in JSON format. JSON is
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

## HdfData

HDF5 data saving code. Additional info to follow.

## Visual

Modern OpenGL visualisation code.

## HexGrid

A class to manage simulations carried out on hexagonal grids with
arbitrary boundaries.

## Scale

A class for data scaling, with autoscaling features.

## Vector

An extension of std::array to make a class for mathematical vector
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

## BezCurve, BezCurvePath and BezCoord

Classes to create Bezier curves.

## CMakeLists.txt for your project

To include a description of writing your own CMakeLists.txt file.
