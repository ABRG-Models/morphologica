# Building with morphologica on GNU/Linux

You don't need to *build* morphologica to use the headers, but
you *will* need to install the dependencies.

The cmake-driven morphologica build process compiles a set of test and
example programs which require all of the dependencies to be met.

Programs that ```#include``` morphologica headers will also need to link to
some or all of those dependencies. Finally, you'll need the cmake
program and a C++ compiler which can compile c++-17 code.

## *Required*: Install dependencies

morphologica code depends on OpenGL, Freetype and glfw3. Armadillo and HDF5 are optional dependencies which you may need. Armadillo is required if you use the BezCurve class or any of the classes HexGrid/ReadCurve/CartGrid (which all use BezCurvePath and hence BezCurve). HDF5 is required if you use the HdfData wrapper class, or if you want to compile HexGrid/CartGrid with built-in save() and load() functions.

### Package-managed dependencies for Ubuntu/Debian

To install the visualization dependencies on Ubuntu or Debian Linux:

```sh
sudo apt install build-essential cmake git \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev

```
For the optional dependencies it's:
```sh
sudo apt install libarmadillo-dev libhdf5-dev libopencv-dev qtcreator
```
Armadillo is only required if you use the ```morph::BezCurve``` class. The HDF5 library is required if you use the wrapper class ```morph::HdfData``` or any of the classes that make use of HdfData (```HexGrid```,```CartGrid```,```Anneal```,```DirichDom```,```RecurrentNetworkModel```,```RD_Base``` and ```DirichVtx```). Their tests and examples should all compile if the libraries are detected and be omitted if not. OpenCV is only used to compile some test/example programs, and these should be omitted from the build process if OpenCV is not detected. Installing qtcreator will bring in the Qt5 libraries that are used to compile Qt-morphologica example programs. It is probably easy to install just the Qt5 Core, Gui and Widgets libraries that are used, but that's how I did it!

### Package-managed dependencies for Arch Linux

On Arch Linux, all required dependencies except Armadillo are available in the official repository. They can be installed as follows:

```shell
sudo pacman -S vtk lapack blas freeglut glfw-wayland
# Optional:
sudo pacman -S hdf5
```

**Note:** Specify `glfw-x11` instead of `glfw-wayland` if you use X.org.

Then, optionally, install [Armadillo](https://aur.archlinux.org/packages/armadillo/) from AUR.

## *Optional*: Build morphologica

To build the morphologica tests, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
# If you have doxygen, you can build the docs with -DBUILD_DOC=1.
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
# I usually place the morphologica directory inside the code repository I'm working
# on, I call this 'in-tree morphologica', but you can also have the headers in
# /usr/local/include if you install:
sudo make install
```

If you need to build the test programs with a specific compiler, such
as g++-7 or clang, then you just change the cmake call in the recipe
above. It becomes:

```sh
CC=gcc-7 CXX=g++-7 cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
```

### Build the client code

See the top level README for a quick description of how to include morphologica in your client code.

### Tests

To run the test suite, use the `ctest` command in the build directory.

Note that certain test cases will fail if no display server is available (e.g. in Docker images). See also [issue #6](https://github.com/ABRG-Models/morphologica/issues/6).


## Hints for older systems

On **Ubuntu 18.04**, I always installed HDF5, jsoncpp and glfw3 from source, rather than using packaged versions.

On **Ubuntu 16.04**, the packaged cmake is too old, so please manually download and install a recent cmake from https://cmake.org/

morphologica requires a fairly up to date compiler. The one on Ubuntu 16.04 is not supported. Obtain a recent stable gcc (version 7.x, 8.x or 9.x).

If you're on Ubuntu 16.04 (or otherwise find you need GLEW), make sure you have libglew (sudo apt install libglew-dev).
You'll then need to add the switch -DUSE_GLEW=ON when calling cmake.

### Building some of the dependencies manually

Only necessary if the package managed libs don't work.

#### HDF5 library build

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

#### JSON library build

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

#### glfw3 library build

The modern OpenGL code in Visual/HexGridVisual requires the library GLFW3.

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

## Docker

A minimal Docker image based on [Alpine Linux](https://alpinelinux.org/) can be created as follows:

```
cd morphologica/docker/
docker build .
```
