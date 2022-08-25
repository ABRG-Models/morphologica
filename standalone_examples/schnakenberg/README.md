# Schakenberg reaction diffusion system

This is a standalone example of a program which builds against
morphologica. It's distributed in a sub-directory within morphologica,
but the morphologica build process won't build schakenberg - you have
to create a build directory right here, and do an independent
cmake/make process.

```bash
# Clone an in-tree morphologica:
git clone git@github.com:ABRG-Models/morphologica

# OR symlink a morphologica instance that you already have:
# ln -s path/to/morphologica # makes morphologica headers available

mkdir build && pushd build
cmake ..
make
popd
./build/schnakenberg ./schakenberg.json
```
