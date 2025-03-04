---
title: morph::VisualResources
parent: Internal classes
grand_parent: Reference
permalink: /ref/visualint/visualresources
layout: page
nav_order: 1
---
```c++
#include <morph/VisualResourcesMX.h> // default multicontext code
// or
#include <morph/VisualResources.h>   // alternative single context code
```

`morph::VisualResources` manages per-program resources. It manages the FreeType fonts in your program (by managing a map of `morph::gl::VisualFace` objects).

This class is a morphologica-internal class and there is typically no
access of its methods in morphologica client code.

`VisualResourcesMX` is a singleton class, accessed via the static
instance function `VisualResourcesMX::i()`. It comes into existence when
the first `morph::Visual` accesses it for the first time in a program.

Freetype library management occurs in
`VisualResourcesMX::freetype_init(Visual<>*)`. Originally I believed
that I required only a single Freetype library instance (allocated
with the function `FT_Init_FreeType`). This was the reason for carrying
out Freetype library init within the VisualResources class. However, I
found it necessary to initialize one Freetype library instance for
*each* `morph::Visual`. This suggests that a better design would be
for `morph::Visual` to own the freetype library pointer/memory. For
now, Freetype init remains in `VisualResources`. The Freetype library
instances are stored in a member attribute which is a map of pointers:
`VisualResourcesMX::freetypes`. This map uses the `Visual` instance
address pointers as a key.

The only other member attribute is `faces` which maps unique_ptrs to [`morph::gl::VisualFaceMX`](/morphologica/ref/visual/visualface) instances with a key which is a tuple of a font identifier ([`morph::VisualFont`](/morphologica/ref/visual/visualface#visualfont)), a font texture resolution and a pointer to the relevant `Visual`.
