# Raspberry Pi examples

There are some of the original examples in this directory which have been slightly modified so that they will run on a Raspberry Pi.

If you diff **examples/pi/convolve.cpp** with **examples/convolve.cpp** you'll see that the only difference is to specify the version `morph::gl::version_3_1_es` as a template parameter for `morph::Visual` and for any VisualModels.

On your Pi, follow the usual build process

```bash
sudo apt install build-essential cmake git \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libglfw3-dev libfreetype-dev

git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
cmake ..
make convolve_pi graph1_pi hexgrid_pi
./examples/pi/graph1_pi
```
