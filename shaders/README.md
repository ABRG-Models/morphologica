These shader files are for development/testing. Shaders for
morph::Visual are then compiled into the binary.

To use one of these file-based shaders (so you can hack around with
it) just copy the files into the same directory that you're launching
your program from.

For example:

```
cd build
cp ../shaders/Visual.vert.glsl .
cp ../shaders/Visual.frag.glsl .
./examples/hexgrid // will load shader progs from the .glsl files
```
