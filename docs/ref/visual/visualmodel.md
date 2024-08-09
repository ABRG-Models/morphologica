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
`morph::Visual` scene. This page describes the base class. There is a
whole [section](/morphologica/ref/visualmodels) dedicated to
documenting all the VisualModel-derived classes that morphologica
provides. These are a great resource for learning how to write your
own VisualModels.

## Basic flow for creating an instance of a VisualModel-derived class

## Adding text labels to VisualModels

## Initializing Vertices

## Re-initializing a VisualModel

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

vao, vbos, indices, vertexPositions, vertexNormals, vertexColours

## Methods used when saving glTF files
