---
title: morph::VisualModel
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/visualmodel
layout: page
nav_order: 2
---
# `VisualModel`: The base class for objects
{: .no_toc}

```c++
#include <morph/VisualModel.h>
```
**Table of Contents**

- TOC
{:toc}

# Overview

`morph::VisualModel` is the base class for graphical objects in your
`morph::Visual` scene. This page describes the base class and then
there is a [section](/morphologica/ref/visualmodels) dedicated to
documenting the derived classes that morphologica provides.

![Four VisualModels in a Visual scene. Each model is a geodesic polynomial](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/geodesics.png?raw=true)

A `VisualModel` holds all the coordinates that define a set of
triangles that make up a 'graphical model' (in OpenGL we draw almost
exclusively with triangles). `VisualModel` also contains a list of
text objects so that your graphical elements can be embellished with
text. Text is created by drawing rectangles (made from triangles) to
which bitmap 'texture' images of character glyphs are applied. The image above shows geodesic polynomials which are constructed from triangles. Here, each triangle is individually coloured so you can see the OpenGL model structure clearly.

# Creating an instance

Taking a derived class called `GraphVisual` as an example, we create an instance of a `VisualModel`-derived class by using `std::make_unique`. This allows us to pass ownership of the VisualModel's memory into a `morph::Visual`. Our example first needs a `morph::Visual` instance:

```c++
morph::Visual v(1024, 768, "Example");
```
The new VisualModel (a GraphVisual) must be created with `std::make_unique`
```c++
auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
```
Once we have the `unique_ptr`, we have to call the essential bindmodel function to wire up the callbacks:
```c++
v.bindmodel (gv);
```
After bindmodel, programs will then make any additional function calls to set up the object. Here we might set the data and change what kind of axes should be drawn.

After set up, the `VisualModel::finalize()` function is the last piece of boilerplate code.
`VisualModel::finalize` is as essential as `bindmodel`. `finalize`
calls the virtual function `VisualModel::initializeVertices` which is
defined as empty (rather than abstract) in `VisualModel`, but must be
overridden in derived classes. The job of `initializeVertices` is to
create the vertices, normals, colours and indices that fill the OpenGL
vertex buffers.

```c++
gv->finalize();
```
Lastly, we transfer memory ownership to the parent Visual with `Visual::addVisualModel()`.
```c++
morph::GraphVisual<float>* gv_ptr = v.addVisualModel (gv);
```
In the example, `Visual::bindmodel` is used to set up these callback functions in VisualModel:

```c++
template <int glver = morph::gl::version_4_1>
class VisualModel
{
    ...
    //! Get all shader progs
    std::function<morph::visgl::visual_shaderprogs(morph::Visual<glver>*)> get_shaderprogs;
    //! Get the graphics shader prog id
    std::function<GLuint(morph::Visual<glver>*)> get_gprog;
    //! Get the text shader prog id
    std::function<GLuint(morph::Visual<glver>*)> get_tprog;
    //! Set OpenGL context. Should call parentVis->setContext()
    std::function<void(morph::Visual<glver>*)> setContext;
    ...
};
```

Three of the functions ensure that the `VisualModel` can get access to the OpenGL shader program IDs in a way that avoids a circular header dependency between Visual.h and VisualModel.h. `VisualModel::render()` needs access to the shader information. The last function allows `VisualModel` to set the correct OpenGL context in any of its function calls that use the OpenGL library code.

# Initializing Vertices

`initializeVertices` implementations will fill these `vector` VisualModel members:
```c++
//! CPU-side data for indices
std::vector<GLuint> indices;
//! CPU-side data for vertex positions
std::vector<float> vertexPositions;
//! CPU-side data for vertex normals
std::vector<float> vertexNormals;
//! CPU-side data for vertex colours
std::vector<float> vertexColors;
```

Once initializeVertices has returned, `finalize` calls
`VisualModel::postVertexInit`, which essentially copies the data in
the vectors into the OpenGL context, using the OpenGL attributes
`VisualModel::vao` and `VisualModel::vbos`:

```c++
//! The OpenGL Vertex Array Object
GLuint vao;
//! Vertex Buffer Objects stored in an array
std::unique_ptr<GLuint[]> vbos;
```

Each triplet of elements in vertexPositions defines a coordinate in
model space. There are an equal number of vertexNormal vectors and of
vertexColors. `indices` is filled with triplets of indices into
`vertexPositions` that define triangles. To write an
initializeVertices implementation, you simply need to create the
vertex coordinates and a suitable sequence of indices. Here's about as
simple an example as possible; a single triangle:

```c++
void initializeVertices()
{
    // First clear out vertexPositions, etc:
    this->clear();
    // Now draw a triangle from three vertices
    vec<float> v1 = { 0, 0, 0 };
    vec<float> v2 = { 1, 0, 0 };
    vec<float> v3 = { 0, 1, 0 };
    // Compute the face normal
    vec<float> u1 = v1 - v2;
    vec<float> u2 = v2 - v3;
    vec<float> face_normal = u1.cross (u2);
    face_normal.renormalize();
    // Push three corner vertices
    this->vertex_push (v1, this->vertexPositions);
    this->vertex_push (v2, this->vertexPositions);
    this->vertex_push (v3, this->vertexPositions);
    // Push corresponding colours/normals
    for (size_t i = 0; i < 3; ++i) {
        this->vertex_push (morph::colour::crimson, this->vertexColors);
        this->vertex_push (face_normal, this->vertexNormals);
    }
    // Push indices
    this->indices.push_back (this->idx++); // Pushes 0
    this->indices.push_back (this->idx++); // Pushes 1
    this->indices.push_back (this->idx++); // Pushes 2
    // At end, idx has the value 3
}
```

You may not need to fill the vertices and indices manually in every
case. If you wish to build a model from rods, cones, discs and
spheres, then you can use the VisualModel graphics primitives.

If you need to create a custom visualization, then you may well end up
needing to write your own initializeVertices function. Consult other
examples in the graphics primitives for more inspiration.

# Re-initializing a VisualModel

If you need your VisualModel to *change* then you will need to
reinitialize it. There are several reinit functions for different
purposes:

`void reinit()` clears out your vertexPositions (etc) and then calls
`initilizeVertices` afresh. If your initializeVertices function uses
data, and the data has changed, the new graphical model will reflect
the changes in the data. `reinit` does *not* clear out any text
labels. For that you need `reinit_with_clearTexts()`.

`reinit_colour_buffer` allows you to update only the
vertexColors. Where you are using only colour to indicate changing
values, this can be a very efficient way of updating your
visualization.

# The VisualModel coordinate frame

When you add vertices to a VisualModel, you do so in the model's own
frame of reference.  The VisualModel coordinate frame uses the same
arbitrary unit of length as the scene's coordinate frame.

The VisualModel class holds two transformation matrices (a model view
matrix and a scene view matrix) which are applied in the shader when
the models are rendered in the scene. The scene view matrix changes as
the scene view point is altered (by mouse events). The model view
matrix is a way to rotate the model's coordinate frame independently.

# Adding text labels to VisualModels

The VisualModel base class provides `addLabel` methods to add texts to
your models.

```c++
visualModel_ptr->addLabel ("Label text", {0, -1, 0}, morph::TextFeatures(0.06f));
```

Here, the first argument is simply a `const std::string&` of the text you want to display. The second argument is a coordinate in the model frame which defines where the text should appear and the third argument is a [`TextFeatures`](/morphologica/ref/visual/textfeatures) object that defines font size, face, resolution and colour. Usually, you construct `TextFeatures` in the `addLabel` arguments.

With the simplest TextFeatures constructor, you set only the font size to 0.06 (`vm_ptr` is a `VisualModel*` pointer):
```c++
vm_ptr->addLabel ("Label text", {0, -1, 0}, morph::TextFeatures(0.06f));
```
You can set the text colour with a second argument in a two-argument constructor. Colours can be chosen from the [`morph::colour`](/morphologica/ref/visual/colour) namespace.
```c++
vm_ptr->addLabel ("Large text", {0, -1, 0}, morph::TextFeatures(0.12f, morph::colour::crimson));
```
Another two-argument constructor allows you to set the font size and the resolution:
```c++
vm_ptr->addLabel ("High-res text", {0, -1, 0}, morph::TextFeatures(0.3f, 128));
```

There is a three argument constructor to set font size, font resolution and font colour. The font resolution here is the resolution of the bitmap image for the font glyph. You have to adjust this based on the size of the font; it it is too high or too low, your fonts will not look very good on screen.

```c++
vm_ptr->addLabel ("Small text", {0, -1, 0}, morph::TextFeatures(0.03f, 48, morph::colour::red));
```

The font face can be chosen like this with the 5 argument constructor. Available font faces are found in the enum class `morph::VisualFont` in [`VisualFace.h`](/morphologica/ref/visualint/visualface#available-font-faces)
```c++
bool no_centering = false;
vm_ptr->addLabel ("Small text", {0, -1, 0}, morph::TextFeatures(0.03f, 48, no_centering,
                                                                morph::colour::blue1,
                                                                morph::VisualFont::DVSansItalic));
```
When you add a text to a VisualModel is is created as a `VisualTextModel` and added to a member attribute which is a `vector` of texts. This is a vector of unique pointers, indicating that the `VisualModel` owns the memory associated with the texts.

```c++
template <int glver = morph::gl::version_4_1>
class VisualModel
{
    ...
    //! A vector of pointers to text models that should be rendered.
    std::vector<std::unique_ptr<morph::VisualTextModel<glver>>> texts;
    ...
};
```
By default, the text will have its vertical axis aligned with the model coordinate frame's 'y' axis and the horizontal axis is aligned with the 'x' axis. It is possible to change this by rotating the text models (see [`morph::GraphVisual::drawAxisLabels`](/morphologica/ref/visualmodels/graphvisual) for example code; the coordinate axis labels in [CoordArrows](/morphologica/ref/visualmodels/coordarrows) also rotate).

## Adding text with symbols

You can incorporate a variety of symbols in your text, including Greek
characters and mathematical symbols. This is achieved with the help of
[`morph::unicode`](/morphologica/ref/visual/unicode) and the
[`VisualFont::DejaVu`](/morphologica/ref/visualint/visualface) that
provides a wide range of non-Latin Glyphs.

```c++
std::string spc(", ");
std::string greek = "Greek ABC: "
                    + morph::unicode::toUtf8 (morph::unicode::alpha)
                    + spc + morph::unicode::toUtf8 (morph::unicode::beta)
                    + spc + morph::unicode::toUtf8 (morph::unicode::gamma);
vm_ptr->addLabel (greek, {0,0,0});
```

`morph::unicode::alpha` is a `constexpr char32_t` containing the
unicode value for the alpha character which is
0x03b1. `morph::unicode::toUtf8` is a static function that converts
this 32 bit character code into a sequence of 8 bit wide UTF-8
codes. These UTF-8 codes can be appended to your `std::string` and
passed straight into `VisualModel::addLabel`. You can output the UTF-8
to a modern command line, too.

# `VisualModel` features

There are a number of features built into `VisualModel`, including the
transparency of the model, whether it is currently visible and whether
it should be allowed to rotate.

## Setting the alpha channel

The `setAlpha` function allows you to set the transparency of a VisualModel. The single argument is the alpha value, with 0 giving a fully transparent (i.e. invisible) model  and 1 giving a fully opaque model (the default).

```c++
vm_ptr->setAlpha (0.8f); // valid range: [0, 1]
```

## Hiding a model

It is sometimes useful to hide a model in a scene. This is carried out with `setHide(bool)` and `toggleHide()`.

```c++
vm_ptr->setHide();      // Hide the vm_ptr model
vm_ptr->setHide(false); // Un-hide the model
vm_ptr->toggleHide();   // Toggle hiddenness
```

## Scaling the model

The function `VisualModel::setSizeScale(float)` sets up a transformation matrix `VisualModel::model_scaling` which is multiplied by the view matrix on each call to `render()`. The argument to setSizeScale scales the model equally in all directions by a scalar factor.

## Two dimensional models

OpenGL is fundamentally three dimensional. However, some models, such as a `GraphVisual` are only really useful as two dimensional figures. It's possible to prevent a model from being rotatable within the scene, even when other models *can* rotate. Use the Boolean `VisualModel::twodimensional` attribute to indicate which.

```c++
cube_vm_ptr->twodimensional = false; // This cube won't rotate
```

You can change this attribute at any time, it will be used on each call to `render()`. In some derived classes, such as  `GraphVisual`,  `twodimensional` is set to true by default.

# Graphics primitives

## Tubes

Rods or tubes can be created with the `computeTube` functions:
```c++
void computeTube (vec<float> start, vec<float> end,
                  std::array<float, 3> colStart, std::array<float, 3> colEnd,
                  float r = 1.0f, int segments = 12)
```

Here we specify start and end coordinates for the tube, along with start and end colours, a tube radius and the number of segments to draw the tube. With segments set to 4, you will get a square section tube.

![Screenshot of rods/tubes](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/rods.png?raw=true)

Example code to generate the image above is in [examples/rod.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/rod.cpp).

Arrows are tube-like ([`VectorVisual`](https://github.com/ABRG-Models/morphologica/blob/main/morph/VectorVisual.h) makes use of this, see [examples/vectorvis.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/vectorvis.cpp)):
```c++
void computeArrow (const vec<float>& start, const vec<float>& end,
                   const std::array<float, 3> clr,
                   float tube_radius = -1.0f,
                   float arrowhead_prop = -1.0f,
                   float cone_radius = -1.0f,
                   const int shapesides = 18)
```
There is also a flared tube:
```c++
void computeFlaredTube (morph::vec<float> start, morph::vec<float> end,
                        std::array<float, 3> colStart, std::array<float, 3> colEnd,
                        float r = 1.0f, int segments = 12, float flare = 0.0f)
```

## Lines

If a tube is the wrong kind of line for your visualization, you may need a 'flat line'. We have `VisualModel::computeFlatLine` and friends.

## Cones

Use `computeCone` to draw a cone. Provide coordinates of the centre of the cone base, the tip along with colour, radius, number of segments to draw with and finally a ringoffset, the effect of which is obvious in the image below (it offsets the circle of the cone away from the base so that the cone effectively becomes a double cone with two tips).

```c++
void computeCone (vec<float> centre, vec<float> tip,
                  float ringoffset, std::array<float, 3> col, float r = 1.0f, int segments = 12)
```

![Screenshot of the computeCone example](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/A_simple_cone.png?raw=true)

Example computeCone code: [examples/cone.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/cone.cpp)

## Spheres

There are a couple of different sphere primitives. `computeSphere` draws a fan of triangles at each end, then fills in the space with rings of triangles. The image below shows also `computeSphereGeo` which computes an icosahedral geodesic to pattern the triangles.
```c++
float x = 1.2f;
this->computeSphere (morph::vec<float>{-x, 0.0f, 0.0f }, morph::colour::royalblue, 1.0f, 12, 12);
// These compute the sphere from a geodesic icosahedron. First with 2 triangulation iterations
this->computeSphereGeo (morph::vec<float>{ x, 0.0f, 0.0f }, morph::colour::maroon, 1.0f, 2);
float y = x * std::tan (morph::mathconst<float>::pi_over_3);
// This one with 3 iterations (meaning more triangles and a smoother sphere) and compile-time geodesic computation
this->computeSphereGeoFast (morph::vec<float>{ 0.0f, y, 0.0f }, morph::colour::cyan3, 1.0f, 3);
```
![Screenshot of spheres](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/Sphere_primitives.png?raw=true)

[examples/sphere.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/sphere.cpp) generated the image above.

## Rings

`computeRing` draws a ring made of flat quads. Example is [examples/ring.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/ring.cpp).

![Screenshot of rings](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/rings.png?raw=true)

## Discs

Thick discs can be made with very short tubes (use `computeTube`). There is also a function to make 2D polygons: `computeFlatPoly`.

```c++
this->computeFlatPoly (vec<float> vstart,
                       vec<float> _ux, vec<float> _uy,
                       std::array<float, 3> col,
                       float r = 1.0f, int segments = 12, float rotation = 0.0f)
```

This'll need some explanation.

# Protected attributes

vao, vbos, indices, vertexPositions, vertexNormals, vertexColors

# Methods used when saving glTF files
