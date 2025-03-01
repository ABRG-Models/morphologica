# Building with Visual Studio

It is possible to build the examples and test programs using Visual Studio.

You first have to install vcpkg.

You open the cmake folder from Visual Studio, and make use of [vcpkg](https://github.com/microsoft/vcpkg) to get dependencies in place. There is a vcpkg.json file in the repository at [vcpkg/vcpkg.json](https://github.com/ABRG-Models/morphologica/blob/main/vcpkg/vcpkg.json).

# Building on Windows Subsystem for Linux

WSL is able to compile a program with OpenGL and display the graphics. In a recent (Feb 2025) test, it appeared to use software rendering, so the frame rate was slow, but, it *did* work.
