---
title: VisualCommon.h
parent: Internal classes
grand_parent: Reference
permalink: /ref/visualint/visualcommon
layout: page
nav_order: 2
---
```c++
#include <morph/VisualCommon.h>
```
This header contains a couple of structs used across the `Visual` and `VisualModel` classes. These are enclosed within a `morph::visgl` namespace to indicate that they are related to OpenGL graphics and related to `morph::Visual`.

## Shader program struct: `visgl::visual_shaderprogs`

```c++
struct visual_shaderprogs
{
    //! An OpenGL shader program for graphical objects
    unsigned int /*GLuint*/ gprog = 0;
    //! A text shader program, which uses textures to draw text on quads.
    unsigned int /*GLuint*/ tprog = 0;
};
```

This is a tiny struct that contains the 32 bit unsigned integers that are used
in OpenGL to identify your shader programs. Currently, `morph::Visual`
needs to retain two shader programs; one for shading your
`VisualModels` and one for shading your text.

`morph::Visual` has a member of type `visual_shaderprogs` and this
must be made available to each `VisualModel` before it can execute its
`render()` method.

## Enumerated class: `visgl::graphics_shader_type`

```c++
enum class graphics_shader_type
{
    none,         // Unset/unknown graphics shader type
    projection2d, // both orthographic and perspective projections to a 2D surface
    cylindrical,  // A cylindrical projection
    spherical     // not implemented, but we could have a spherical projection
};
```

An enumerated class to refer to different shader types. This is actually used only in `morph::Visual` to distinguish between orthographic/projection shaders (which can use a common shader) and cylindrical or other shaders, which likely require a different GLSL shader program.

## Enum: `visgl::AttribLocn`
```c++
//! The locations for the position, normal and colour vertex attributes in the
//! morph::Visual GLSL programs
enum AttribLocn { posnLoc = 0, normLoc = 1, colLoc = 2, textureLoc = 3 };
```
An enumerated type used in the set up of OpenGL vertex buffer objects. Used in `morph::VisualTextModel` and `morph::VisualModel`.

## Glyph information struct: `visgl::CharInfo`

```c++
struct CharInfo
{
    //! ID handle of the glyph texture
    unsigned int textureID;
    //! Size of glyph
    morph::vec<int,2>  size;
    //! Offset from baseline to left/top of glyph
    morph::vec<int,2>  bearing;
    //! Offset to advance to next glyph
    unsigned int advance;
};
```
A struct that contains font glyph properties which are loaded with the Freetype library (in `morph::VisualFace`). The properties are then accessed when text is to be rendered in `morph::VisualTextModel`.