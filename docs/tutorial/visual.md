---
title: morph::Visual
parent: Tutorials
permalink: /tutorials/visual
layout: page
nav_order: 2
---
# Getting started with `morph::Visual`

[`morph::Visual`](https://github.com/ABRG-Models/morphologica/blob/main/morph/Visual.h) is the graphics scene class. It bundles up all the complexities of OpenGL rendering, including that of managing fonts for text into a single header-only include. It
allows you to pan and zoom the environment to view 3D
visualisations from any angle. It bundles fonts and text-handling code
to allow you to render anti-aliased text in your models. It provides
shaders that give you lighting and alpha-blend effects. It also
provides the facilities to save an image of the scene, or save the
graphical portions of the scene into a
[glTF](https://github.com/KhronosGroup/glTF) file (so that you could
open them in Blender). If you want to make a custom visualisation for
a simulation, the only job you have to do is to describe what shapes
will make up the graphical model. And because it's *modern* OpenGL,
the graphics rendering is really fast, so you can visualise your
simulations in realtime.

## Creating a scene

Here's a "Helloworld" example that creates a `morph::Visual` object, adds a label containing some text at coordinates (0,0,0) in the scene, and renders it in a window.

```c++
#include <morph/Visual.h>
int main()
{
    // Visual is templated with an int parameter that defines the OpenGL version
    morph::Visual<morph::gl::version_4_1>  v(600, 400, "Hello World!");
    v.addLabel ("Hello World!", {0,0,0});
    v.keepOpen(); // Renders the scene and polls for mouse/keyboard events
    return 0;
}
```

This program is in morphologica's examples, so you can
compile and run it with:

```bash
cd morphologica
mkdir build
cd build
cmake ..
make helloworld
./examples/helloworld
```
It's an empty Visual scene with a text label and nothing
else. However, try pressing 'Ctrl-c' in the window, and you'll see the
3D coordinate system arrows appear. Press 'Ctrl-q' to exit.

## Adding a visual model to the scene
Rod
A Visual scene is nothing without some objects inside it. We'll use an example VisualModel called RodVisual to add an object. Add this include to the helloworld program:
```c++
#include <morph/RodVisual.h>
```
Then before v.keepOpen() you can add a simple VisualModel:

```c++
morph::vec<float, 3> pos = { 0.0, 0.0, 0.0 };     // model position in scene
morph::vec<float, 3> start = { 0, 0, 0 };         // start coordinate of rod (model frame of reference)
morph::vec<float, 3> end = { 0.25, 0, 0 };        // end coordinate of rod
morph::vec<float, 3> colour1 = { 1.0, 0.0, 0.0 }; // Colour for the rod
// This will return a std::unique_ptr<morph::VisualModel<>> into rvm. VisualModels
// have to have the gl version template arg.
auto rvm = std::make_unique<morph::RodVisual<morph::gl::version_4_1>> (pos, start, end, 0.1f, colour1, colour1);
v.bindmodel (rvm);      // boilerplate for all VisualModels
// Any model specific settings, such as rvm->setWidth(0.4f); would go here
rvm->finalize();        // Required. Causes the OpenGL vertices to be computed
auto rvm_ptr = v.addVisualModel (rvm); // Transfer ownership of the model into the morph::Visual
```
The pattern is to:
* create a `std::unique_ptr` to the VisualModel with `std::make_unique`
* call Visual's `bindmodel()` method, passing in the VisualModel unique_ptr. This sets some runtime callbacks
* set any features of the model (colour, dimensions, associated data etc)
* `finalize()` the model, which causes the vertices to be computed (The final derived version of `VisualModel::initalizeVertices()` is called to do this)
* add it to the `morph::Visual` with `addVisualModel()`, transferring ownership of the unique_ptr.

Note that `addVisualModel` returns a non-owning pointer to the model, which can be used to interact with the model after it has been added to the Visual. In the example above, the `auto` type will be determined to be `morph::RodVisual<morph::gl::version_4_1>*`.

## Render the scene and poll for events

The `Visual::render` method calls render() for each object in the scene and re-draws the entire window. You can call render() as often as is necessary in your program, but be sure to include a call to waitevents() so that your keyboard/mouse input can be processed. A common pattern would be:
```c++
MyModel model;
while (!v.readyToFinish) {
    // Do computations on your model object
    model.step(); // This may have nothing to do with morphologica
    if ((model.stepnum % 100) == 0) { // Render every 100 steps
        // Calls to update VisualModels and then...
        v.render();
        v.waitevents (0.001); // A very short wait
    }
}
```
There is also `v.poll()` if you want to poll without any timeout or `v.wait(double timeout)` if you want to wait a full timeout (`waitevents` returns as soon as there is a keyboard, window resize or mouse event *or* when the timeout elapses).

If you don't need to change the scene you're viewing, then
```c++
v.keepOpen();
```
is equivalent to `render()` followed by `waitevents (0.018)` in an infinite loop.


### OpenGL Context

It is essential that the correct OpenGL context is current when rendering a morph::Visual. This is especially true when rendering to two Visual windows in one program, or when rendering in one morph::Visual window, while another does GL-compute operations.

In these situations, before calling `Visual::render` for a Visual object, call its setContext() method:

```c++
v.setContext();
v.render();
```
If you only have one window, you won't need to setContext().

## The scene view
The scene will render VisualModel objects at different coordinates. You may add several graphs on a grid, and when you've completed the scene, you will want the new window to appear with a view that places all the graphs within a appropriately sized window. You can set the window size in the Visual constructor, but to move the 'view' of the scene, you need `Visual::setSceneTrans`. This translates the entire scene by a 3D vector. By moving the scene in the z coordinate, you can zoom.
```c++
v.setSceneTrans (morph::vec<float,3>({0.0f, 0.0f, -5.0f}));
```
To get the right numbers for the offset, Visual has a neat trick which you can try with any of the example programs. With the mouse, pan and zoom the content until it looks right. Press 'Ctrl-z' and then look in the stdout for the program which should look something like:

```
Scenetrans setup code:
    v.setSceneTrans (morph::vec<float,3>({-0.411793f, -0.397834f, -5.0f}));
scene rotation is Quaternion[wxyz]=(0.999999,0.00121815,0,0)
Writing scene trans/rotation into /tmp/Visual.json... Success.
```

It will give the setSceneTrans() call you need. Add it after the Visual constructor:
```c++
morph::Visual<morph::gl::version_4_1> v(600, 400, "Hello World!");
v.setSceneTrans (morph::vec<float,3>({-0.411793f, -0.397834f, -5.0f}));
```

## Background colour and lighting

You can set the background colour to anything by changing Visual::bgcolour which is of type std::array<float, 4> (RGBA). To set white or black backgrounds, it's just
```c++
v.backgroundWhite();
v.backgroundBlack();
```

Often in a visualization you don't want any kind of lighting effects, because an exact colour conveys information, so any kind of lighting effect distorts the information perceived by the viewer. However, to add a simple diffuse light source (which switches morph::Visual to use a slightly different set of shaders) you can call
```c++
v.lightingEffects (true);
```

## Perspective

The default is to render with perspective. However, it's possible to choose an orthgraphic projection by changing the `Visual::ptype` attribute
```c++
// Change from default of morph::perspective_type::perspective;
v.ptype = morph::perspective_type::orthographic;
```

## Saving

You can save a PNG image of the view with
```c++
v.saveImage ("file_path.png");
```
or a glTF file that describes the graphical elements of your scene (excluding text) with

```c++
v.savegltf ("file_path.gltf");
```

## Callbacks

morph::Visual contains a scheme for setting mouse and keyboard callbacks so that you can add custom key press events to your programs. See [Custom callbacks in extended Visual classes](/custom_callbacks) for more details.

## Removing a VisualModel

You may need to remove a VisualModel from your scene. Use `removeVisualModel`:

```c++
auto rvm = std::make_unique<morph::RodVisual<morph::gl::version_4_1>> (pos, start, end, 0.1f, colour1, colour1);
auto rvm_ptr = v.addVisualModel (rvm);
// Stuff happens
v.removeVisualModel (rvm_ptr);
rvm_ptr = nullptr; // Don't use it again until it points to something valid
```