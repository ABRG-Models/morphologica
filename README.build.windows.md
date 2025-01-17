# Building with Visual Studio

In late 2021 I ported morphologica code to run on Windows, because I
wanted to be able to port [Stalefish](https://github.com/ABRG-Models/Stalefish) to Windows (although I didn't end up doing this).

After making a number of modifications for Windows-compatibility, I successfully compiled the morphologica test and example programs with Visual Studio 2019, using the CMake directed build process.
I managed to install the dependencies and compile all the examples and tests (bar one or two that are turned off for Windows).
The OpenGL code works well.
I know of at least one other user who compiled a morphologica program on Windows and you should be able to as well.
However, note that there is no Windows-based Github action for continuous integration so it is possible that Windows incompatibilities have crept in since 2021.

Installing the dependencies is left as an exercise for the Windows-savvy reader.
I managed to do this on my test system, but I don't have a good sense of what best practice should be.
In common with Mac and Linux, you will need OpenGL (this was present on my machine already), Freetype (I git cloned this at commit 38b349c) and glfw3 (git cloned at 56a4cb0a).
Optional libraries are HDF5 (I used the hdf5-1.10.7-Std-win10_64-vs16 installer), Armadillo (I used version 10.7.1; Armadillo is only required to compile the Bezier curve test programs, you don't need it for the visualization code) and OpenCV (git cloned at b5a9a679).
In 2021 it was necessary to compile jsoncpp (git cloned at 94a6220f) but now morphologica contains a header-only json library.
For my Windows build, I also found it necessary to install the GL extension wrangler library, glew (I used version 2.2.0).

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

**Note:** Examples involving HexGrids are often **VERY slow**.
It appears that some of the code I wrote for setting up a large HexGrid is orders of magnitude slower on Windows than on Linux and Mac.
I don't know why this is and I don't really have any reason to fix it, as I don't run numerically intensive programs involving HexGrids on Windows (feel free to contribute a pull request).
Other than this issue, the programs all seem to work, even text in OpenGL!

## CMakeSettings.json

This is the settings file that allowed me to run CMake within Visual Studio to attempt to compile morphologica code on Windows.

```json
{
  "note" : "This is the settings file that allowed me to run CMake within Visual Studio to attempt to compile morphologica code on Windows.",

  "configurations": [
    {
      "buildCommandArgs": "",
      "buildRoot": "${projectDir}\\out\\build\\${name}",
      "cmakeCommandArgs": "-DCMAKE_PREFIX_PATH=C:/Users/seb/source/repos/jsoncpp/out/install/x64-Debug/lib/cmake/jsoncpp;C:/Users/seb/Source/Repos/glfw/out/install/x64-Debug/lib/cmake/glfw3/;C:/Users/seb/source/repos/opencv/out/install/x64-Debug/x64/vc16/lib;C:/Users/seb/source/repos/armadillo-10.7.1/out/install/x64-Debug;C:/Users/seb/source/repos/CMake-hdf5-1.10.7/hdf5-1.10.7/out/install/x64-Debug/cmake/hdf5/;C:/Users/seb/source/repos/freetype/out/install/x64-Debug",
      "configurationType": "Debug",
      "ctestCommandArgs": "",
      "generator": "Ninja",
      "inheritEnvironments": [ "msvc_x64_x64" ],
      "installRoot": "${projectDir}\\out\\install\\${name}",
      "name": "x64-Debug"
    }
  ]
}
```

## cmake commands used on Windows

There are some more hints at how I built targets on Windows:

```
cmake --build  . --target hexgrid

after


cmake -DCMAKE_PREFIX_PATH="C:/Users/Seb James/source/repos/jsoncpp/out/install/x64-Debug/lib/cmake/jsoncpp;C:/Users/Seb James/Source/Repos/glfw/out/install/x64-Debug/lib/cmake/glfw3/;C:/Users/Seb James/source/repos/opencv/out/install/x64-Debug/x64/vc16/lib;C:/Users/Seb James/source/repos/armadillo-10.7.1/out/install/x64-Debug;C:/Users/Seb James/source/repos/CMake-hdf5-1.10.7/hdf5-1.10.7/out/install/x64-Debug/cmake/hdf5/;C:/Users/Seb James/source/repos/freetype/out/install/x64-Debug" ..


cmake -DCMAKE_PREFIX_PATH="C:/Users/Seb James/source/repos/jsoncpp/out/install/x64-Debug/lib/cmake/jsoncpp;\
C:/Users/Seb James/Source/Repos/glfw/out/install/x64-Debug;\
C:/Users/Seb James/source/repos/opencv/out/install/x64-Debug/x64/vc16/lib;
C:/Users/Seb James/source/repos/armadillo-10.7.1/out/install/x64-Debug;
C:/Users/Seb James/source/repos/CMake-hdf5-1.10.7/hdf5-1.10.7/out/install/x64-Debug;
C:/Users/Seb James/source/repos/freetype/out/install/x64-Debug" ..



'C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.29.30133\bin\HostX64\x64\link.exe' /ERRORREPORT:QUEUE /OUT:"C:\Users\Seb James\source\repos\morphologica\build\examples\Debug\hexgrid.exe" /INCREMENTAL /ILK:"hexgrid.dir\Debug\hexgrid.ilk" /NOLOGO "C:\Users\Seb James\source\repos\armadillo-10.7.1\out\install\x64-Debug\lib\armadillo.lib" "C:\Users\Seb James\source\repos\armadillo-10.7.1\out\install\x64-Debug\lib\armadillo.lib" "C:\Users\Seb James\source\repos\glfw\out\install\x64-Debug\lib\glfw3.lib" "C:\Users\Seb James\source\repos\freetype\out\install\x64-Debug\lib\freetyped.lib" "C:\Users\Seb James\source\repos\jsoncpp\out\install\x64-Debug\lib\jsoncpp.lib" opengl32.lib glu32.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib /MANIFEST /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /manifest:embed /DEBUG /PDB:"C:/Users/Seb James/source/repos/morphologica/build/examples/Debug/hexgrid.pdb" /SUBSYSTEM:CONSOLE /BID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"C:/Users/Seb James/source/repos/morphologica/build/examples/Debug/hexgrid.lib" /MACHINE:X64  /machine:x64 hexgrid.dir\Debug\hexgrid.obj

```

# Building on Windows Subsystem for Linux

While it was possible to compile and build morphologica on Windows subsystem for Linux in 2021, none of the OpenGL graphics were available.
Everything else should work as normal.
