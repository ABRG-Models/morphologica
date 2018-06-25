# morphologica

Library code used in models developed by Stuart P. Wilson and co-workers

This code builds a shared library called libmorphologica.

It installs the library on your system, along with the required header
files.

Code is (or shortly will be) enclosed in the "morph" namespace.

It requires OpenCV, Armadillo and OpenGL headers to compile, and
programs linked with libmorphologica will also need to link to those
dependencies.

To build, it's the usual CMake process:

```
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
Linux, that means running ldconfig, as above, assuming you had the
library installed in /usr/local, as in the example above. If you
installed elsewhere, then you probably know how to set
LD\_CONFIG\_PATH correctly.
