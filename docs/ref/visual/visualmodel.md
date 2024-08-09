---
title: morph::VisualModel
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/visualmodel
layout: page
nav_order: 2
---
```c++
#include <morph/VisualModel.h>
```

`morph::VisualModel` is the base class for graphical models in your
`morph::Visual` scene. This page describes the base class. (There is a
whole [section](/morphologica/ref/visualmodels) dedicated to
documenting all the VisualModel-derived classes that morphologica
provides, which are a great resource for learning how to write your
own VisualModels.)

A `VisualModel` holds all the coordinates that define a set of
triangles that make up a 'graphical model'. You can draw anything with
triangles and besides this, OpenGL gives you almost no other way to
draw. `VisualModel` also contains a list of text objects so that your
graphical elements can be embellished with text. Text is created by
drawing rectangles (made from triangles) to which bitmap 'texture'
images of character glyphs are applied.

## Creating a VisualModel instance

Taking a derived class called `GraphVisual` as an example, we create an instance of a `VisualModel`-derived class by using `std::make_unique`. This allows us to pass ownership of the VisualModel's memory into a `morph::Visual`.

```c++
morph::Visual v(1024, 768, "Example");
// Create a new VisualModel
auto gv = std::make_unique<morph::GraphVisual<float>> (morph::vec<float>({0,0,0}));
// Call essential bindmodel function to wire up callbacks
v.bindmodel (gv);
// [snip] Call additional methods from GraphVisual to set it up further
// Call VisualModel::finalize to build the OpenGL model
gv->finalize();
// Transfer memory ownership to the parent Visual
morph::GraphVisual<float>* gv_ptr = v.addVisualModel (gv);
```
we call `Visual::bindmodel` to set up these callback functions in VisualModel:

```c++
//! Get all shader progs
std::function<morph::visgl::visual_shaderprogs(morph::Visual<glver>*)> get_shaderprogs;
//! Get the graphics shader prog id
std::function<GLuint(morph::Visual<glver>*)> get_gprog;
//! Get the text shader prog id
std::function<GLuint(morph::Visual<glver>*)> get_tprog;
//! Set OpenGL context. Should call parentVis->setContext()
std::function<void(morph::Visual<glver>*)> setContext;
```

The example omits any GraphVisual-specific setup, but normally this would appear before `gv->finalize()`.

`VisualModel::finalize` is as essential as `bindmodel`. `finalize`
calls the virtual function `VisualModel::initializeVertices` which is
defined as empty (rather than abstract) in `VisualModel`, but must be
overridden in derived classes. The job of `initializeVertices` is to
create the vertices, normals, colours and indices that fill the OpenGL
vertex buffers.

## Initializing Vertices

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
needing to write you own initializeVertices function. Consult other
examples in the graphics primitives for more inspiration.

## Re-initializing a VisualModel

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

## Adding text labels to VisualModels

`VisualModel::addLabel`...

## Features you can change

`setAlpha` `setHide` `toggleHide` `setSizeScale` `twodimensional`

## Graphics primitives

### Tubes

Rods or tubes can be created with the `computeTube` functions:
```c++
void computeTube (vec<float> start, vec<float> end,
                  std::array<float, 3> colStart, std::array<float, 3> colEnd,
                  float r = 1.0f, int segments = 12)
```

Here we specify start and end coordinates for the tube, along with start and end colours, a tube radius and the number of segments to draw the tube. With segments set to 4, you will get a square section tube.

![Screenshot of rods/tubes](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/rods.png?raw=true)


Arrows are tube-like:
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
### Cones

Use `computeCone` to draw a cone. Provide coordinates of the centre of the cone base, the tip along with colour, radius, number of segments to draw with and finally a ringoffset, the effect of which is obvious in the image below (it offsets the circle of the cone away from the base so that the cone effectively becomes a double cone with two tips).

```c++
void computeCone (vec<float> centre, vec<float> tip,
                  float ringoffset, std::array<float, 3> col, float r = 1.0f, int segments = 12)
```

![Screenshot of the computeCone example](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/A_simple_cone.png?raw=true)

### Spheres

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

### Rings

### Discs

## Protected attributes

vao, vbos, indices, vertexPositions, vertexNormals, vertexColors

## Methods used when saving glTF files
