# wxWidgets examples

Here we have some example programs showing how to make sure of `morph::Visual` for drawing OpenGL graphics within a wxWindows/wxWidgets program.

The build process on Linux is typical, but we do need libglew and libwxgtk3.2 (wx 3.1+ is required, so Ubuntu versions 23.04 and up will work).

```bash
sudo apt install build-essential cmake git \
                 freeglut3-dev libglu1-mesa-dev libxmu-dev libxi-dev \
                 libfreetype-dev libglew-dev libwxgtk3.2-dev

git clone https://github.com/ABRG-Models/morphologica.git
cd morphologica
mkdir build
cd build
cmake -DUSE_GLEW=ON .. # You have to switch the use of GLEW on with wxWidgets
make wx-graph1 wx-graph6 wx-triaxes
./examples/pi/wx-graph1
```
