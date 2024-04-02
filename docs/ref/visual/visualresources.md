---
title: morph::VisualResources
parent: Visualization classes
grand_parent: Reference
permalink: /ref/visual/visualresources
layout: page
nav_order: 1
---
```c++
#include <morph/VisualResources.h>
```
`morph::VisualResources` manages per-program resources. It handles some GLFW window setup and it also manages the FreeType fonts in your program (by managing a map of `morph::gl::VisualFace` objects).