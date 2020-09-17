# Build and Install morphologica on Apple Mac

The cmake-driven morphologica build & install process installs static
and shared object libraries on your system, along with the required
header files.

It requires OpenCV, Armadillo, OpenGL, HDF5, LAPACK and X headers to
compile, and programs linked with libmorphologica will also need to
link to those dependencies. You will also need the cmake program and a
C++ compiler which can compile c++-17 code.

## Installation dependencies for Mac

You will need XQuartz, XCode and Mac Ports. Install XQuartz from
http://xquartz.org/ and XCode from the App Store. If you just
installed XCode, then you'll need to agree to its licence terms. To do
this, run

```
sudo xcodebuild -license
```

scroll through the legalese and type 'agree' (assuming that you
do). Also, do make sure to run XCode at least once from the launcher
as this will prompt it to download and install some additional
components. Finally, it seems to be necessary to "install command line
tools" to get a working compiler. To do so (at least on MacOS
Catalina):

```
xcode-select --install
```

Installation of most of the other dependencies can be achieved using Mac
ports. This does lead to the installation of a great deal of
additional software, some of which can conflict with Mac system
software (that's libiconv, in particular). However, a clean install of
Mac ports will successfully install the dependencies for
morphologica. I'd advise you to use Option 1: Mac Ports only if you
*already* use Mac Ports, other wise, prefer Option 2: Manual
dependency builds.

### Option 1: Mac Ports

Install Mac ports, following the instructions on
http://www.macports.org/. This will guide you to install the XCode
command line tools, then install the Mac ports installation package.

Finally, use Mac ports to install the rest of the dependencies:

```sh
sudo port install cmake armadillo opencv
```

*Be aware that if you have conflicting versions of any of the
 libraries in another location (such as /usr/local), you may run into
 problems during the build.*

### Option 2: Manual dependency builds

It's much cleaner to build each of the dependencies by hand. That
means first installing cmake, which I do with a binary package from
https://cmake.org/download/, and then compiling hdf5, opencv and
armadillo (all of which support a cmake build process).


#### Armadillo

Armadillo is a library for matrix manipulation. One place it's used in
morphologica is within the Bezier curve code,
morph::BezCurve. Download a package - I downloaded
armadillo-9.900.3.tar.xz, though older versions back to 8.400.0 should
work.

```sh
mkdir -p ~/src
cd ~/src
tar xvf path/to/downloaded/armadillo-9.900.3.tar.xz
cd armadillo-9.900.3
mkdir build
cd build
cmake ..
make
sudo make install
```

#### HDF5

Hierarchical data format is a standard format for saving binary
data. morph::HdfData wraps the HDF5 API and hence HDF5 is a required
dependency to build morphologica. Build version 1.10.x.


```sh
mkdir -p ~/src
cd ~/src
tar xvf path/to/downloaded/hdf5-1.10.7.tar.gz
cd hdf5-1.10.7
mkdir build
cd build
cmake ..
make
sudo make install
```

#### OpenCV

Computer vision. Used to save views of the OpenGL
environment. morph::HdfData is also OpenCV-aware (and can save
cv::Points and cv::Mats). I compiled OpenCV master from git that's
probably about version 4.4.0. OpenCV versions as old as 3.2.0 also work.

```sh
mkdir -p ~/src
cd ~/src
git clone git@github.com:opencv/opencv.git
cd opencv
mkdir build
cd build
cmake ..
make
sudo make install
```

### Common manual dependency builds

Whether or not you used mac ports to install hdf5, opencv and
armadillo, the JSON-reading library jsoncpp needs to be built
separately, as I don't believe it is currently available as a
port. glfw3 also needs to be built separately (I've not investigated
whether there is a mac ports version of glfw).

#### jsoncpp

jsoncpp allows morph::Config to read from and write to JSON text
files. Compile and install jsoncpp in /usr/local like this:

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

#### glfw3

The modern OpenGL code in morphologica requires the GL-window managing
library GLFW3 and only compiles if GLFW3 is present. Compile it like
this:

```
cd ~/src
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build
cmake ..
make
sudo make install
```

## Build morphologica on Mac

To build morphologica itself, it's the usual CMake process:

```sh
cd ~/src
git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
# If you have doxygen, you can build the docs with -DBUILD_DOC=1.
# If you have OpenMP, you can add -DCOMPILE_WITH_OPENMP=1
cmake ..
make -j$(nproc)
sudo make install
```

## Tests
To run the test suite, use the `ctest` command in the build directory.
