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
                 libglfw3-dev libfreetype-dev nlohmann-json3-dev

```
For the optional dependencies it's:
```sh
sudo apt install libarmadillo-dev libhdf5-dev qtcreator libwxgtk3.2-dev libgbm-dev libegl-dev
```
* Armadillo. Only required if you use the ```morph::BezCurve``` class.
* HDF5 library. Required if you use the wrapper class ```morph::HdfData``` or any of the classes that make use of HdfData (```HexGrid```,```CartGrid```,```Anneal```,```DirichDom```,```RecurrentNetworkModel```,```RD_Base``` and ```DirichVtx```). Their tests and examples should all compile if the libraries are detected and be omitted if not.
* Qt library. Installing qtcreator will bring in the Qt5 libraries that are used to compile some Qt-morphologica example programs. It almost certainly possible to install *only* the Qt5 Core, Gui and Widgets libraries, but that hasn't been verified.
* WxWindows. libwxgtk3.2-dev (you'll need Ubuntu 23.04+) will enable the compilation of morphologica-wxWidgets example programs.
* GBM. Required only for window-less OpenGL compute compilations. Currently that's one example program only.
* EGL. Requried to build GLES applications that are compatible with Raspberry Pi 4 and 5.

### Package-managed dependencies for Arch Linux

On Arch Linux, all required dependencies except Armadillo are available in the official repository. They can be installed as follows:

```shell
sudo pacman -S vtk lapack blas freeglut glfw-wayland nlohmann-json
# Optional:
sudo pacman -S hdf5
```

**Note:** Specify `glfw-x11` instead of `glfw-wayland` if you use X.org.

Then, optionally, install [Armadillo](https://aur.archlinux.org/packages/armadillo/) from AUR.

## *Optional*: Build morphologica examples or tests

To build the morphologica example programs, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
cmake ..
make -j$(nproc)
# I usually place the morphologica directory inside the code repository I'm working
# on, I call this 'in-tree morphologica', but you can also have the headers in
# /usr/local/include (control location with the usual CMAKE_INSTALL_PREFIX) if you install:
# sudo make install
```
### Building test programs (or NOT building the examples)

By default, the example programs are built with the call to `make`, but unit test programs are not. To build test programs, and control whether example programs are compiled, use the cmake flags `BUILD_TESTS` and `BUILD_EXAMPLES`, changing your cmake line to:
```sh
cmake .. -DBUILD_TESTS=ON -DBUILD_EXAMPLES=OFF # Build tests but not examples
# ...etc
```

If you need to build the test programs with a specific compiler, such
as g++-7 or clang, then you just change the cmake call in the recipe
above. It becomes:

```sh
CC=gcc-7 CXX=g++-7 cmake .. -DBUILD_TESTS=ON
```
To run the test suite, use the `ctest` command in the build directory or `make test`.

### Build the client code

See the top level README for a quick description of how to include morphologica in your client code and [README.cmake.md] for more information.

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

(Not currently maintained) A minimal Docker image based on [Alpine Linux](https://alpinelinux.org/) can be created as follows:

```
cd morphologica/docker/
docker build .
```
