# Build and Install morphologica on GNU/Linux

The cmake-driven morphologica build & install process installs the
header files and font files that are required to build against
morphologica. Some libraries may also be installed.

The build process also compiles a set of test programs, which require
OpenCV, Armadillo, OpenGL, Freetype, glfw, HDF5 and LAPACK to compile.
Programs that include morphologica headers will also need to link to
some or all of those dependencies. Finally, you'll need the cmake
program and a C++ compiler which can compile c++-17 code.

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

### glfw3 library

The modern OpenGL code in Visual/HexGridVisual requires the library
GLFW3.

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

Note that this is required only if you are building morphologica with
glfw. When building on Ubuntu 16.04 issue#13 showed up. To work
around, I added a link to libglew.so and a call to glewInit() in
morph::Visual. Because this is unnecessary on other platforms (Ubuntu
18/19 and Mac) I made it an option in the cmake build process.

If you're on Ubuntu 16.04 (or otherwise find you need GLEW), make sure
you have libglew:

```
sudo apt install libglew-dev
```

You'll then need to add the switch -DUSE_GLEW=ON when calling cmake.

## Build morphologica

To build morphologica tests, it's the usual CMake process:

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
# ldconfig is only necessary for the legacy library libmorphdisplay0.so
sudo ldconfig
```

If you need to build the test programs with a specific compiler, such
as g++-7 or clang, then you just change the cmake call in the recipe
above. It becomes:

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

#### Build the client code

See the top level README for a quick description of how to include
morphologica in your client code.

## Tests

To run the test suite, use the `ctest` command in the build directory.

Note that certain test cases will fail if no display server is available (e.g. in Docker images). See also [issue #6](https://github.com/ABRG-Models/morphologica/issues/6).

## Docker

A minimal Docker image based on [Alpine Linux](https://alpinelinux.org/) can be created as follows:

```
cd morphologica/docker/
docker build .
```
