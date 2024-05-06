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

*This is the [twowindows.cpp](https://github.com/ABRG-Models/morphologica/blob/main/examples/twowindows.cpp) example program, which displays two windows with two `morph::Visual` instances.*

## Background: the `morph::Visual` GL version and `OWNED_MODE`

### OpenGL Version

When you program with OpenGL, you have to choose which of the many versions of the library you want to use. This software uses 'modern OpenGL' which essentially means that we draw with GLSL shader programs. Modern OpenGL starts with OpenGL version 3.3, but version 4.1 was chosen as a starting point as this is supported across the Linux, Mac and Windows platforms.

OpenGL 4.1 was originally the only option, but more recently Visual and friends were extended to support other OpenGL versions, including OpenGL 4.1 to 4.6 (which makes it possible to use GL compute shaders) and OpenGL 3.0 ES and up, which makes it possible to run morphologica programs on the Raspberry Pi.

The OpenGL version is passed to `morph::Visual` as a single template argument `glver` of type `int`.

The default value for `glver` is `morph::gl::version_4_1` which requests the core version 4.1 of OpenGL. The integer values that specify each OpenGL version are defined in [morph/gl/version.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/gl/version.h). Both the 'desktop' OpenGL versions (from 4.1 up to 4.6) and the 'ES' versions (3.0 ES to 3.2 ES) are supported in both core and compatibility modes.

The OpenGL version integer is also used as a template parameter in the `morph::VisualModel` objects that will populate your `morph::Visual`. You should ensure that the same value for the GL version is used across all classes.

### `OWNED_MODE`

One more concept to introduce before getting into `morph::Visual` usage is the 'operating mode'. When Visual was first developed, it was designed to own its desktop window, which would always be provided by the [GLFW library](https://www.glfw.org/). The Visual class would manage GLFW setup and window creation/destruction. Window pointers (aliased as `morph::win_t`) were simply of type `GLFWwindow`.

Later on, I wanted to add support for the Qt windowing system so that a morph::Visual could provide OpenGL graphics for a QtWidget. Qt manages OpenGL contexts and windows, so I had to create a new operating mode for `morph::Visual` in which it would use an externally managed context. To do this I defined `OWNED_MODE`. `OWNED_MODE` is enabled by `#define OWNED_MODE 1` in a relevant location (see [viswidget.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/qt/viswidget.h) for Qt and [viswx.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/wx/viswx.h) for wx windows).

In owned mode, an alternative windowing system can be used and `morph::win_t` is mapped to the appropriate window/widget type. Code that is involved in setting up the windowing system is disabled.

However, unless you are integrating morphologica into Qt or WxWidgets, you will leave `OWNED_MODE` undefined.

## Instantiating `morph::Visual`
