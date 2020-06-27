# morphologica

Library code used in models developed by Stuart P. Wilson, Seb James
and co-workers in the Wilson Lab.

This c++ code builds a shared library called libmorphologica which
contains **simulation support facilities** for our simulations of
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

Below, I've added a quick-start guide to the main classes.

### morph::Config

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
    string logpath = conf.getString ("logpath", "fromfilename");
}
```
At the end of your program, it's a good idea to write out a copy of
the params.json file into your log directory, along with some
information about the simulation run:

```c++
{
    // Add information into the existing morph::Config object:
    conf.set ("float_width", (unsigned int)sizeof(FLT)); // FLT is a template param
    string tnow = morph::Tools::timeNow();
    conf.set ("sim_ran_at_time", tnow);
    if (argc > 0) { conf.set("argv0", argv[0]); }
    if (argc > 1) { conf.set("argv1", argv[1]); }
    conf.insertGitInfo ("sim/"); // checks 'sim' dir for any post-commit changes
    const string paramsCopy = logpath + "/params.json";
    conf.write (paramsCopy);
    if (conf.ready == false) { /* handle error */ }
}
```

### HdfData

HDF5 data saving code. Additional info to follow.

### Visual

Modern OpenGL visualisation code.

### HexGrid

A class to manage simulations carried out on hexagonal grids with
arbitrary boundaries.

### Scale

A class for data scaling, with autoscaling features.

### Vector

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

### BezCurve, BezCurvePath and BezCoord

Classes to create Bezier curves.

## Installation

The cmake-driven morphologica build & install process installs static
and shared object libraries on your system, along with the required
header files.

It requires OpenCV, Armadillo, OpenGL, HDF5, LAPACK and X headers to compile, and
programs linked with libmorphologica will also need to link to those
dependencies. You will also need the cmake program and a C++ compiler
which can compile c++-17 code.

## Installation dependencies for GNU/Linux

### Package-managed dependencies for Ubuntu/Debian

To install the necessary dependencies on Ubuntu or Debian Linux, start with:

```sh
sudo apt install build-essential cmake git libopencv-dev libarmadillo-dev \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev liblapack-dev wget
```

### Package-managed dependencies for Arch Linux

On Arch Linux, all required dependencies except Armadillo are available in the official repository. They can be installed as follows:

```shell
sudo pacman -S vtk hdf5 lapack blas freeglut jsoncpp glfw-wayland
```

**Note:** Specify `glfw-x11` instead of `glfw-wayland` if you use X.org.

Then, install [Armadillo](https://aur.archlinux.org/packages/armadillo/) from AUR.

### cmake for older systems (if required)

On **Ubuntu 16.04**, the packaged cmake is too old to compile hdf5-1.10.x. On this OS (or others with cmake version <3.10), please manually download and install a recent cmake from https://cmake.org/

```sh
mkdir -p ~/src
cd ~/src
cp path/to/cmake-3.16.4.tar.gz ./ # any cmake version 3.10 or higher should be ok
gunzip cmake-3.16.4.tar.gz
tar xvf path/to/cmake-3.16.4.tar
cd cmake-3.16.4
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_USE_OPENSSL=OFF
make -j$(nproc)
sudo make install
```

### gcc/g++ for older systems (if required)

morphologica requires a fairly up to date compiler. The one on Ubuntu 16.04 is not supported. Download and build a recent stable gcc (version 7.x, 8.x or 9.x). Alternatively, on Ubuntu 16.04 you can:

```sh
sudo add-apt-repository ppa:jonathonf/gcc-7.1
sudo apt update
sudo apt install gcc-7 g++-7
```

### HDF5 library

You will also need HDF5 installed on your system. There _is_ an HDF5 package for Ubuntu, but I couldn't get the morphologica cmake build process to find it nicely, so I compiled my own version of HDF5 and installed in /usr/local. To do what I did, download HDF5 (https://portal.hdfgroup.org/display/support/Downloads), and do a compile and install like this:

```sh
mkdir -p ~/src
cd ~/src
cp path/to/hdf5-1.10.x.tar.gz ./
gunzip hdf5-1.10.x.tar.gz
tar xvf hdf5-1.10.x.tar
cd hdf5-1.10.x
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

### armadillo for older systems (if required)

On Ubuntu 16.04, the packaged armadillo is too old, so first remove that:

```sh
sudo apt remove libarmadillo-dev
sudo apt autoremove
sudo apt install libopenblas-dev
```

Then download and compile an up-to-date version

```sh
cd ~/src
wget http://sourceforge.net/projects/arma/files/armadillo-9.850.1.tar.xz
tar xf armadillo-9.850.1.tar.xz # On some platforms you may need to do multiple steps here
cd armadillo-9.850.1
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

### JSON library

For the saving and reading of configuration information, you'll need
the jsoncpp library compiled and installed in /usr/local. I cloned it
from github:

```sh
cd ~/src
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
mkdir build
cd build
cmake ..
make
sudo make install
```

This installs jsoncpp as a static library in
/usr/local/lib/libjsoncpp.a which is then linked directly to
libmorphologica by means of the src/CMakeLists.txt file. I'm using the
HEAD of the master branch of the jsoncpp repository, which installs a
library with version about 1.8.4.

### glfw3 library (Optional)

There is some OpenGL 2 style OpenGL code in display.h/cpp and also
some more modern OpenGL code in Visual/HexGridVisual. This modern code
requires the library GLFW3 and only compiles if GLFW3 is present.

It's possible to apt install glfw on recent versions of Ubuntu. Doing so
will install libglfw.a. These build instructions install libglfw3.a (into
/usr/local/lib/).

```
sudo apt install libxinerama-dev libxrandr-dev libxcursor-dev
cd ~/src
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
cmake ..
make
sudo make install
```
#### libglew on some systems (if required)

Note that this is required only if you are building morphologica with glfw. When building on Ubuntu 16.04 issue#13 showed up. To work around, I added a link to libglew.so and a call to glewInit() in morph::Visual. Because this is unnecessary on other platforms (Ubuntu 18/19 and Mac) I made it an option in the cmake build process.

If you're on Ubuntu 16.04 (or otherwise find you need GLEW), make sure you have libglew:
```
sudo apt install libglew-dev
```
You'll then need to add the switch -DUSE_GLEW=ON when calling cmake.

## Installation dependencies for Mac

You will need XQuartz, XCode and Mac Ports. Install XQuartz from http://xquartz.org/ and XCode from the App Store.

Installation of the other dependencies is best achieved using Mac ports. Install Mac ports, following the instructions on http://www.macports.org/. This will guide you to install the XCode command line tools, then install the Mac ports installation package.

Finally, use Mac ports to install the rest of the dependencies:

```sh
sudo port install cmake armadillo opencv
```

Note that the installation of armadillo will pull in hdf5, so on Mac, we don't need to compile hdf5 manually.

*Be aware that if you have conflicting versions of any of the
 libraries, you may run into problems during the build.*

You'll need jsoncpp on Mac, too, built as a static library, just like for Linux, above:

```sh
mkdir -p ~/src
cd ~/src
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
mkdir build
cd build
cmake ..
make
sudo make install
```
Also make sure that PKG_CONFIG_PATH environment variable is set up correctly, some Mac ports systems don't have this set up and then jsoncpp isn't found. Add

```
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```
to your .bashrc.


### (Optional) Install glfw3

The modern OpenGL code in morphologica requires the library GLFW3 and only compiles if
GLFW3 is present.

```
# NB: Untested on Mac as yet!
cd ~/src
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
cmake ..
make
sudo make install
```

## Installation on Windows

While it is possible to compile and build morphologica on Windows (by using Windows subsystem for Linux to provide a Linux environment) I haven't had it fully working myself, and don't support it or suggest that you try. 

If you *really* want to try here are some hints, but you'll have to solve the OpenGL-4.1-on-WSL problem that I wasn't able to.

### How to fail to fully install morphologica on Windows with Windows subsytem for Linux

To install on Windows, first install *Windows subsystem for Linux* https://docs.microsoft.com/en-us/windows/wsl/install-win10

Install the Ubuntu 18.04 image from the Windows store. Fully upgrade all packages before you start:

```sh
sudo apt-get install apt
sudo apt update
sudo apt upgrade
```

Now you can follow instructions for installing on GNU/Linux, above, to get some of the morphologica code to compile.

In principle, if you install an X server on your Windows desktop, then the graphical morphologica programs should be compilable. This is required for some of the morphologica test programs to run. I tried Xming, and I payed a donation to the developer to get the up-to-date version of Xming (the free version will definitely not work for modern OpenGL applications). In my Windows subsystem for Linux Ubuntu 18.04 environment I exported the DISPLAY environment variable so that programs will know where to draw their output:

```sh
export DISPLAY=:0
```
(actually, I added this to my .bashrc).

However, Xming version 7.7 didn't work for me (morphologica requires OpenGL version 4.1). I then tried VcXsrv, but that didn't work for me either.

It *may* be possible to get one or other of these to work, but I gave up.

## Docker

A minimal Docker image based on [Alpine Linux](https://alpinelinux.org/) can be created as follows:

```
cd morphologica/docker/
docker build .
```

## Build morphologica

To build morphologica, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
# If you have doxygen, you can build the docs with -DBUILD_DOC=1.
# If you have OpenMP, you can remove the COMPILE_WITH_OPENMP option or set it to 1
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_DOC=0 -DCOMPILE_WITH_OPENMP=0
make -j$(nproc)
sudo make install
sudo ldconfig # Probably Linux specific! Mac alternative?
```

Note the call to ldconfig at the end there, which makes sure that
libmorphologica is available to your system's dynamic linker. On
Linux, that means running ldconfig (assuming that the
CMAKE_INSTALL_PREFIX of /usr/local is already in your dynamic
linker's search path) as in the example above. If you installed
elsewhere, then you probably know how to set LD_LIBRARY_PATH
correctly (or see **Building/installing as a per-user library**, below).

If you need to build with a specific compiler, such as g++-7 or clang,
then you just change the cmake call in the recipe above. It becomes:

```sh
CC=gcc-7 CXX=g++-7 cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

On Ubuntu 16.04, you'll also need to USE_GLEW:

```sh
CC=gcc-7 CXX=g++-7 cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DUSE_GLEW=ON
```


If necessary, you can pass
-DMORPH_ARMADILLO_LIBPATH=/usr/local/lib and the linker will add this
before -larmadillo during linking

### Building/installing as a per-user library

#### Build morphologica:

(Make sure you are on a version of morphologica later than the master
branch of 2:15 PM, Jan 27 2020, as I added an important line to
pc/CMakeLists.txt).

```bash
cd ~/src/morphologica/build
cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/usr
make -j$(nproc)
make install # no sudo! You don't need it to install in your home
```

#### Update the environment

Edit ${HOME}/.bashrc and add:

```bash
# This line because libmorphologica.so is installed in ${HOME}/usr/lib/
export LD_LIBRARY_PATH=${HOME}/usr/lib:${LD_LIBRARY_PATH}
# This line allows your system's pkg-config program to find your locally
# installed libmorphologica. This allows a program built on morphologica (like
# BarrelEmerge) to find libmorphologica with pkg-config.
export PKG_CONFIG_PATH=${HOME}/usr/lib/pkgconfig:${PKG_CONFIG_PATH}
# This means that any binaries installed in ${HOME}/usr/bin can be executed
# by typing their name into your command line
export PATH=${HOME}/usr/bin:${PATH}
```

Either log out/log in, restart your terminal or type:

```bash
source ${HOME}/.bashrc
```

To get the updated variables into your environment.

#### Build the client code

In the base CMakeLists.txt of, for example,
[BarrelEmerge](https://github.com/ABRG-Models/BarrelEmerge), pkgconfig is
used to find morphologica. This is all that's required to build
against your local libmorphologica. If things aren't working, check
there isn't an alternative morphologica installation (or the pkgconfig
file from an old one). Practically, that means checking there isn't
```
/usr/local/lib/pkgconfig/libmorphologica.pc
```
or
```
/usr/lib/pkgconfig/libmorphologica.pc
```
on your system.

## Tests
To run the test suite, use the `ctest` command in the build directory.

Note that certain test cases will fail if no display server is available (e.g. in Docker images). See also [issue #6](https://github.com/ABRG-Models/morphologica/issues/6).

# Credits

Authorship of morphologica code is given in each file. Copyright in
the software is owned by the authors. morphologica is distributed
under the terms of the GNU General Public License, version 3 (see
LICENSE.txt).
