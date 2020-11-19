# Schakenberg reaction diffusion system

This is a standalone example of a program which builds against
morphologica. It's distributed in a sub-directory within morphologica,
but the morphologica build process won't build schakenberg - you have
to create a build directory right here, and do an independent
cmake/make process.

The default CMakeLists.txt file compiles schakenberg based on the
assumption that you did a make install of the morphologica headers and
they are therefore to be found in /usr/local/include/morph. If you've
done that, then you can build and run schnakenberg like this:

```bash
echo "I assume you installed the morphologica headers..."
mkdir build && pushd build
cmake ..
popd
./build/schnakenberg ./schakenberg.json
```

There is an alternative CMakeLists.txt which can be used with an
'in-tree' copy of morphologica. That's called
CMakeLists_intree.txt. To see how that works do the following:

```bash
cp CMakeLists_intree.txt CMakeLists.txt
# Clone an in-tree morphologica:
git clone git@github.com:ABRG-Models/morphologica
mkdir build && pushd build
cmake ..
popd
./build/schnakenberg ./schakenberg.json
```
