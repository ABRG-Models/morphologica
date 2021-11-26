# Building with Visual Studio

Recently, I ported morphologica code to run on Windows, because I
would like to be able to port [Stalefish](https://github.com/ABRG-Models/Stalefish) to Windows.

The morphologica test and example programs can be compiled with Visual Studio 2019, using the CMake
directed build process. I managed to install the dependencies and
compile all the examples and tests (bar one or two that are turned off
for Windows). The OpenGL code works well.

Installing the dependencies is left as an exercise for the Windows-savvy reader. I managed to do this 
on my test system, but I don't have a good sense of what best practice should be. In common with Mac 
and Linux, you will need Armadillo (I used version 10.7.1), OpenGL (this was present on my machine already), 
Freetype (I git cloned this at commit 38b349c), glfw3 (git cloned at 56a4cb0a), HDF5 (I used the 
hdf5-1.10.7-Std-win10_64-vs16 installer), jsoncpp (git cloned at 94a6220f) and OpenCV (git cloned at b5a9a679). 
I also found it necessary to install the GL extension wrangler library, glew, on Windows (I used version 2.2.0).

I used a cmake build process like this:

```
cd morphologica
mkdir build
cd build

# The long CMAKE_PREFIX_PATH shows where I installed my dependencies, but is almost certainly not best practice!
cmake -DCMAKE_PREFIX_PATH="C:/Users/Seb James/source/repos/jsoncpp/out/install/x64-Debug/lib/cmake/jsoncpp;C:/Users/Seb James/Source/Repos/glfw/out/install/x64-Debug;C:/Users/Seb James/source/repos/opencv/out/install/x64-Debug/x64/vc16/lib;C:/Users/Seb James/source/repos/armadillo-10.7.1/out/install/x64-Debug;C:/Users/Seb James/source/repos/CMake-hdf5-1.10.7/hdf5-1.10.7/out/install/x64-Debug;C:/Users/Seb James/source/repos/freetype/out/install/x64-Debug" .. -DDEBUG_VARIABLES=ON -DUSE_GLEW=ON

cmake --build . -- /m
```

Find test programs in ```build/tests/Debug``` and example programs in ```build/examples/Debug```. A single program can be compiled with a line like:

```
cmake --build . --target graph_bar
```

**Note:** Examples involving HexGrids are often **VERY slow**. It appears
that some of the code I wrote for setting up a large HexGrid is orders
of magnitude slower on Windows than on Linux and Mac. I don't know why this is and I don't really have any
reason to fix it, as I won't be running numerically intensive
programs involving HexGrids on Windows. I would welcome a fix from someone else though - feel free to contribute a pull request! Other than this issue, the
programs all seem to work, even text in OpenGL!

# Building on Windows Subsystem for Linux

While it is possible to compile and build morphologica on Windows
subsystem for Linux, none of the OpenGL graphics will be available to
you. Everything else should work as normal.
