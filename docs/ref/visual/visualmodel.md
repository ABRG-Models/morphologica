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

```c++
void computeTube (vec<float> start, vec<float> end,
                  std::array<float, 3> colStart, std::array<float, 3> colEnd,
                  float r = 1.0f, int segments = 12)
```

Arrows are tube-like:
```c++
void computeArrow (const vec<float>& start, const vec<float>& end,
                   const std::array<float, 3> clr,
                   float tube_radius = -1.0f,
                   float arrowhead_prop = -1.0f,
                   float cone_radius = -1.0f,
                   const int shapesides = 18)
```

```c++
void computeFlaredTube (morph::vec<float> start, morph::vec<float> end,
                        std::array<float, 3> colStart, std::array<float, 3> colEnd,
                        float r = 1.0f, int segments = 12, float flare = 0.0f)
```
### Cones

![Screenshot of the computeCone example](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/A_simple_cone.png?raw=true)

### Spheres

### Rings

### Discs

## Protected attributes

vao, vbos, indices, vertexPositions, vertexNormals, vertexColours

## Methods used when saving glTF files
