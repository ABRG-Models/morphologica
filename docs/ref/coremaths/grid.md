---
layout: page
title: morph::Grid
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/grid/
---
```c++
#include <morph/Grid.h>
```

A Cartesian grid class. `morph::Grid` is a simpler version of [`morph::CartGrid`](https://github.com/ABRG-Models/morphologica/blob/main/morph/CartGrid.h). The difference is that any `morph::Grid` is rectangular, whereas a `morph::CartGrid` may be constructed with an arbitrary domain boundary (`morph::HexGrid` objects can also have an arbitrary boundary). Unless you need non-rectangular boundaries for your Cartesian grids, prefer `morph::Grid` over `morph::CartGrid`.
