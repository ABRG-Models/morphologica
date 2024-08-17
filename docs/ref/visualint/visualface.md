---
title: visgl::VisualFace
parent: Internal classes
grand_parent: Reference
permalink: /ref/visualint/visualface
layout: page
nav_order: 3
---
```c++
#include <morph/VisualFace.h>
```
# Font faces in morphologica

`morph::visgl::VisualFace` contains code that makes TrueType fonts
available for use in morphologica as OpenGL textures (hence the
additional `visgl` namespace).

It contains the following data member attributes:

```c++
//! The FT_Face that we're managingb
FT_Face face;
//! The OpenGL character info stuff
std::map<char32_t, morph::visgl::CharInfo> glchars;
```

It holds a Freetype `face` which specifies the font family and it
populates `glchars` which maps a char, specified in unicode format
(for which the `char32_t` is required) to a `morph::visgl::CharInfo`
object, which holds information about that specific glyph (a textureID
which maps to a bitmap of the character glyph and some dimensional
information; 'size', 'bearing' and 'advance').

VisualFace is constructed with a passed in `morph::VisualFont` which
specifies a supported font such as `VisualFont::DVSans` or
`VisualFont::VeraItalic` along with a texture resolution and a
reference to the Freetype library instance.

In the constructor, the Freetype library is used to generate bitmaps
of each character in the font at the requested resolution. The bitmaps
are saved into the OpenGL context so that they can be used as
textures. glchars is populated at this point with the dimensional
information about each character glyph.

## Available font faces

The available fonts are specified in the enum class `VisualFont` (also
currently in VisualFace.h):

```c++
//! The fonts supported (i.e. compiled in) to morph::Visual
enum class VisualFont {
    DVSans,             // fonts/dejavu/DejaVuSans.ttf
    DVSansItalic,       // fonts/dejavu/DejaVuSans-Oblique.ttf
    DVSansBold,         // fonts/dejavu/DejaVuSans-Bold.ttf
    DVSansBoldItalic,   // fonts/dejavu/DejaVuSans-BoldOblique.ttf
    Vera,               // fonts/ttf-bitstream-vera/Vera.ttf
    VeraItalic,         // fonts/ttf-bitstream-vera/VeraIt.ttf
    VeraBold,           // fonts/ttf-bitstream-vera/VeraBd.ttf
    VeraBoldItalic,     // fonts/ttf-bitstream-vera/VeraBI.ttf
    VeraMono,           // fonts/ttf-bitstream-vera/VeraMono.ttf
    VeraMonoItalic,     // fonts/ttf-bitstream-vera/VeraMoIt.ttf
    VeraMonoBold,       // fonts/ttf-bitstream-vera/VeraMoBD.ttf
    VeraMonoBoldItalic, // fonts/ttf-bitstream-vera/VeraMoBI.ttf
    VeraSerif,          // fonts/ttf-bitstream-vera/VeraSe.ttf
    VeraSerifBold       // fonts/ttf-bitstream-vera/VeraSeBd.ttf
};
```

These fonts are compiled in to each morphologica binary. This makes
morphologica programs more portable, because it is not necessary for
the program to locate a TrueType font at runtime. Instead, the program
*writes out* the TrueType font into a temporary directory, then
directs FreeType to use the temporary font file.

The most useful fonts are the `DVSans` family, as these provide many
unicode characters, including the Greek alphabet.

Although `VisualFace` is not part of the user-facing API, the possible
values of `VisualFont` are used in `morph::TextFeatures` allowing you
to select a font family for your text labels.