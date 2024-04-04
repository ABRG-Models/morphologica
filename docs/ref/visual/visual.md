---
title: morph::Visual
parent: Visualization classes
grand_parent: Reference
permalink: /ref/visual/visual
layout: page
nav_order: 0
---
# morph::Visual
```c++
#include <morph/Visual.h>
```

`morph::Visual<>` manages the graphical scene in which all your
morphologica visualizations will exist.

You need at least one morph::Visual in your program. In general, one
morph::Visual will relate to one window. If you have two or more
Visuals, then you will have two or more windows.

![Screenshot of two computer windows each backed by a morph::Visual](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/morph_two_visuals.png?raw=true)
