# Building with Visual Studio

Recently, I ported morphologica code to run on Windows, because I
would like to be able to port [Stalefish](https://github.com/ABRG-Models/Stalefish) to Windows.

morphologica can be compiled with Visual Studio 2019, using the CMake
directed build process. I managed to install the dependencies and
compile all the examples and tests (bar one or two that are turned off
for Windows). The OpenGL code works well.

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
