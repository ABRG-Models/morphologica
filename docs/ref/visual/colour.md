---
title: morph::colour
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/colour
layout: page
nav_order: 7
---
```c++
#include <morph/colour.h>
```
# Colour naming

`morph::colour` is a namespace full of (static constexpr) named colour
definitions. Colours are specified as 3 element arrays representing
red, green and blue channels. morph::colour provides a list of
pre-defined colours with a naming scheme that follows this web page:

[http://www.cloford.com/resources/colours/500col.htm](http://www.cloford.com/resources/colours/500col.htm)

Each colour is a constexpr `std::array<float, 3>` definition. Where a
function takes a colour specification (such as in a `VisualModel`
primitive) you can pass the named colour like this (using
`VisualModel::computeSphere` as an example):

```c++
// Compute vertices for a mint green sphere
this->computeSphere (sphere_location, morph::colour::mint);
```

I use the webpage to find a colour I want to use, and the names can be used directly as above. The entry corresponding to `morph::colour::mint` is:

![Image of the entry for the colour mint on the cloford.com colour web page](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/mint.png?raw=true)