---
title: morph::TextFeatures
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/textfeatures
layout: page
nav_order: 10
---
```c++
#include <morph/TextFeatures.h>
```

TextFeatures is a small class that holds the attributes required to
specify the appearance of a string of text rendered by morphologica.

It's small enough that the easiest way to document it is to show it!

```c++
// A way to bundle up font size, colour, etc into a single object. Constructors chosen for max convenience.
struct TextFeatures
{
    TextFeatures(){};
    TextFeatures (const float _fontsize,
                  const int _fontres,
                  const bool _centre_horz,
                  const std::array<float, 3> _colour,
                  morph::VisualFont _font)
        : fontsize(_fontsize), fontres(_fontres), centre_horz(_centre_horz), colour(_colour), font(_font) {}

    TextFeatures (const float _fontsize, const bool _centre_horz = false)
        : fontsize(_fontsize)
    {
        this->centre_horz = _centre_horz;
    }

    TextFeatures (const float _fontsize, const std::array<float, 3> _colour, const bool _centre_horz = false)
        : fontsize(_fontsize), colour(_colour)
    {
        this->centre_horz = _centre_horz;
    }

    TextFeatures (const float _fontsize, const int _fontres,
                  const std::array<float, 3> _colour = morph::colour::black, const bool _centre_horz = false)
        : fontsize(_fontsize), fontres(_fontres), colour(_colour)
    {
        this->centre_horz = _centre_horz;
    }

    //! The size for the font
    float fontsize = 0.1f;
    //! The pixel resolution for the font textures
    int fontres = 24;
    //! If true, then centre the text string horizontally
    bool centre_horz = false;
    //! The font colour
    std::array<float, 3> colour = morph::colour::black;
    //! The supported font to use when displaying a text string
    morph::VisualFont font = morph::VisualFont::DVSans;

    // Maybe also things like rotate, centre_vert, etc
};
```

The design and the choice of constructors is intended to enable its
use inline within the argument list of other functions. It's often
passed to `VisualModel::addLabel` like this:

```c++
visualModel_ptr->addLabel ("Label text", {0, -1, 0}, morph::TextFeatures(0.06f));
```

This passes a TextFeature object to `addLabel` which has the default
colour black, the default font `DVSans` (DejaVu Sans) along with the
default font resolution of 24. Only the font size is changed to a
non-default value of 0.06.

This avoids having to pass several additional (and seldom used)
arguments to addLabel. To set other aspects of the font, use one of
the other constructors:

```c++
vm_ptr->addLabel ("Large text", {0, -1, 0}, morph::TextFeatures(0.12f, morph::colour::crimson));
vm_ptr->addLabel ("Small text", {0, -1, 0}, morph::TextFeatures(0.03f, 48, morph::colour::springgreen));
```
