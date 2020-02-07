# morphologica

Library code used in models developed by Stuart P. Wilson and co-workers

This code builds a shared library called libmorphologica.

It installs the library on your system, along with the required header
files.

Code is (or shortly will be) enclosed in the "morph" namespace.

It requires OpenCV, Armadillo, OpenGL and X headers to compile, and
programs linked with libmorphologica will also need to link to those
dependencies. You will also need the cmake program and a C++ compiler.

## Install dependencies on GNU/Linux

To install these dependencies on Ubuntu or Debian Linux, you can do:

```sh
sudo apt install build-essential cmake libopencv-dev libarmadillo-dev \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev
```

You will also need HDF5 installed on your system. There _is_ an HDF5 package for Ubuntu, but I couldn't get the morphologica cmake build process to find it nicely, so I compiled my own version of HDF5 and installed in /usr/local. To do what I did, download HDF5 (https://portal.hdfgroup.org/display/support/Downloads), and do a compile and install like this:

```sh
tar xvf hdf5-1.10.x.tar.gz
cd hdf5-1.10.x
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j4 # or however many cores you have. This takes a while.
sudo make install
```

For the saving and reading of configuration information, you'll need
the jsoncpp library compiled and installed in /usr/local. I cloned it
from github:

```sh
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

There is some OpenGL 2 style OpenGL code in display.h/cpp and also
some more modern OpenGL code in Visual/HexGridVisual. This modern code
requires the library GLFW3 and only compiles if GLFW3 is present.

## Install dependencies on Mac

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
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp
mkdir build
cd build
cmake ..
make
sudo make install
```

## Build morphologica

To build morphologica, it's the usual CMake process:

```sh
cd morphologica
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j4
ctest
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

If necessary, you can pass
-DMORPH_ARMADILLO_LIBPATH=/usr/local/lib and the linker will add this
before -larmadillo during linking

### Building/installing as a per-user library

#### Build morphologica:

(Make sure you are on a version of morphologica later than the master
branch of 2:15 PM, Jan 27 2020, as I added an important line to
pc/CMakeLists.txt).

```bash
cd morphologica
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=${HOME}/usr
make -j4
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
