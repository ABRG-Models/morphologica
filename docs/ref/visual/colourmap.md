---
title: morph::ColourMap
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/colourmap
layout: page
nav_order: 8
---
```c++
#include <morph/ColourMap.h>
```
# Introduction

`morph::ColourMap` provides a set of colour maps to indicate numerical values with graded colours. It's a very important class in the visualization of data! Here is a selection of colour maps (I used [morph::ColourBarVisual](https://github.com/ABRG-Models/morphologica/blob/main/morph/ColourBarVisual.h) to make this graphic):

![A selection of colour maps available in morph::ColourMap](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/ColourMaps.png?raw=true)

The class exists to convert an input number (usually a `float`) in the
range [0, 1] into an RGB colour triplet, returned as (usually)
`std::array<float, 3>`. The examples above show several colour maps
which will be familiar to those who have used colour maps in Python or
MATLAB.

As well as the one dimensional colour maps shown above,
`morph::ColourMap` can convert two (and even three) dimensional
numbers into colours. Here you can see i) the 'HSV' map which converts
the 'x' and 'y' of a two-dimensional input into polar coordinates,
then uses these as the hue (r) and saturation (phi) of an HSV colour
specification and ii) two 'Duochrome' maps that encode two dimensions
of data as complementary colours. Here, we're encoding the 2D coordinate on a square grid into colour.

![A selection of 2D colour maps available in
 morph::ColourMap](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/ColourMaps2D.png?raw=true)

A `ColourMap` is often a member of a VisualModel-derived class. For
example, the
[`HexGridVisual`](/morphologica/ref/visualmodels/hexgridvisual) class
has a `ColourMap` which is used to select colours for each hex in the
grid. These are coloured with the 'plasma' colour map:

![A hex grid with the hexes coloured according to the Plasma colour map](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/hexgrid1.png?raw=true)

# Available maps

The full list of available maps is found in the enumerated class
`morph::ColourMapType`, found in
[ColourMap.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/ColourMap.h):

```c++
enum class ColourMapType // in morph namespace
{
    Jet,
    Rainbow,
    RainbowZeroBlack, // As Rainbow, but if datum is 0, then colour is pure black.
    RainbowZeroWhite, // As Rainbow, but if datum is 0, then colour is pure white.
    Magma,      // Like matplotlib's magma
    Inferno,    // matplotlib's inferno
    Plasma,     // etc
    Viridis,
    Cividis,
    Twilight,
    Greyscale,    // Greyscale is any hue; saturation=0; *value* varies. High signal (datum ->1) gives dark greys to black
    GreyscaleInv, // Inverted Greyscale. High signal gives light greys to white
    Monochrome,   // Monochrome is 'monohue': fixed hue; vary the *saturation* with value fixed at 1.
    MonochromeRed,
    MonochromeBlue,
    MonochromeGreen,
    Monoval,      // Monoval varies the *value* of the colour
    MonovalRed,
    MonovalBlue,
    MonovalGreen,
    Duochrome,    // Two fixed hues, vary saturation of each with two input numbers.
    Trichrome,    // As for Duochrome, but with three inputs
    RGB,          // A kind of 'null' colour map that takes R, G and B values and returns as an RGB colour.
                  // Of course, you don't really need a morph::ColourMap to do this, but it can be useful where
                  // the ColourMap is embedded in the workflow, such as in a VisualDataModel.
    RGBMono,      // Takes RGB input and outputs a coloured monochrome version (datum varies value)
    RGBGrey,      // Takes RGB input and outputs a greyscale version
    HSV,          // A special map in which two input numbers are used to compute a hue and a saturation.
    HSV1D,        // A 1D version of HSV, traverses a line across the HSV circle for a set value of hue,
                  // which determines what colour the value 1 will return. Useful for positive/negative ranges.
                  // Map the negative portion of our input to the rnage 0->0.5 and the positive to 0.5->1
    Fixed         // Fixed colour. Should return same colour for any datum. User must set hue, sat, val.
};
```

All of these maps are one dimensional, converting a scalar (usually in
the range [0, 1]) to a colour. Exceptions are `Duochrome`,
`Trichrome`, `RGB`, `RGBMono`, `RGBGrey` and `HSV` which convert 2 or
3 dimensional vectors into a colour. Lastly, `Fixed` is a special case.
It's a colour map which always returns the same colour, but it will
pretend to be a 1D map.

# Simple usage

## Constructors and `ColourMapType`

Construct with a `ColourMapType` as argument, with a string representation of the colour map type or with no argument:

```c++
// ColourMap::type will default to ColourMapType::Plasma
morph::ColourMap<float> colour_map1;

// Explicitly choose ColourMapType::Plasma:
morph::ColourMap<float> colour_map2(morph::ColourMapType::Plasma);

// Choose using the 'string name':
morph::ColourMap<float> colour_map3("plasma");
```

The string name is always a lower-case version of the ColourMapType
name. Plasma -> "plasma"; GreyscaleInv -> "greyscaleinv" and so on. A
`ColourMapType` can be obtained for a given string name with
the static function `ColourMap::strToColourMapType` (and back again):

```c++
morph::ColourMapType cmtype = morph::ColourMap::strToColourMapType ("jet");
std::string cmtype_str = morph::ColourMap::colourMapTypeToStr (cmtype);
```

You can check the number of datums that the map requires with

```c++
int ndatums = morph::ColourMap::numDatums (morph::ColourMapType::HSV); // returns 2
```

or

```c++
morph::ColourMap<float> colour_map1 (morph::ColourMapType::Twilight);
int ndatums = colour_map1.numDatums(); // returns 1
```

You can set the type after construction and getType() at any time:

```c++
colour_map1.setType (morph::ColourMapType::Jet);
colour_map1.setType (std::string("jet"));
auto thetype = colour_map1.getType();
std::string string_type = colour_map1.getTypeStr();
```

## Accessing colours with `convert()`

Access a colour from the map using the `convert` functions:

```c++
// 1D input maps:
morph::ColourMap<float> colour_map1 (morph::ColourMapType::Viridis);
std::array<float, 3> mycolour1 = colour_map1.convert (0.5f);

// 2D input maps:
morph::ColourMap<float> colour_map2 (morph::ColourMapType::Duochrome);
std::array<float, 3> mycolour2 = colour_map2.convert (0.5f, 0.3f);

// 3D input maps:
morph::ColourMap<float> colour_map3 (morph::ColourMapType::RGBGrey);
std::array<float, 3> mycolour3 = colour_map3.convert (0.5f, 0.3f, 0.2f);
```

If you choose an incompatible `convert` function (such as
convert(float, float) for a 1D ColourMapType) then a runtime error
will be thrown.

## Choice of template type `T`

The examples above show instances of `morph::ColourMap<T>` with
`T=float` which is most commonly used. `double` may also be used. When
the `ColourMap` template type `T` is floating point, inputs should be
given in the range [0, 1].

It is also possible to use integral types with `ColourMap`. In these
cases the range of the input can be between 0 and the maximum value
for the type or 255, whichever is smaller.

You can obtain the maximum value of the range with the `T ColourMap::range_max` variable.

```c++
morph::ColourMap<char> cm_char;
std::array<float, 3> max_colour = cm_char.convert (127);
std::array<float, 3> min_colour = cm_char.convert (0);
std::cout << cm_char.range_max << "\n"; // 127

morph::ColourMap<unsigned char> cm_unsigned_char;
max_colour = cm_unsigned_char.convert (255);
min_colour = cm_unsigned_char.convert (0);
std::cout << cm_unsigned_char.range_max << "\n"; // 255

// Anything with >8 bits uses range [0 255]. Pull requests to change this would be welcome.
morph::ColourMap<unsigned int> cm_unsigned_int;
max_colour = cm_unsigned_int.convert (255);
min_colour = cm_unsigned_int.convert (0);
std::cout << cm_unsigned_int.range_max << "\n"; // 255

// You can even ColourMap<bool>!
morph::ColourMap<bool> cm_bool;
max_colour = cm_bool.convert (true);
min_colour = cm_bool.convert (false);
std::cout << cm_bool.range_max << "\n"; // 1
```

# Using 1D maps

There's not much more to be said about 1D maps; in most cases,
construct a `ColourMap<T>` object, set its `ColourMapType` and then
call `ColourMap::convert(T)`. However, there are some 1D maps which
have some additional options.

## Monochrome and Monoval maps

The `Monochrome` map has a gradually increasing saturation of one
colour. The colour, which defaults to red, is held in the private
member `ColourMap::hue` and can be changed with `setHue`. `Monochrome`
simply wires the input datum to the saturation and creates an HSV
colour based on ( `ColourMap::hue`, input datum, `ColourMap::value`
). `ColourMap::value` is 1 by default, but it can be set with `setVal()`.

```c++
morph::ColourMap<float> colour_map1 (morph::ColourMapType::Monochrome);
colour_map1.setHue (0.3f); // Range [0, 1]. See rainbow map for the hue mapping. 0.3 gives green.
std::array<float, 3> mycolour1 = colour_map1.convert (0.5f);
```
![A selection of Hue variable 1D colour Monochrome maps
 morph::ColourMap](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/monochrome.png?raw=true)

There are some convenience ColourMapTypes that set the hue when you
set the type. These are `MonochromeRed`, `MonochromeGreen` and
`MonochromeBlue`.

The `Monoval` maps are similar to `Monochrome`, but instead of varying
saturation, they vary the value of the HSV colour. They transition
from black to the maximally bright colour. They look like this:

![A selection of Hue variable 1D colour Monoval maps
 morph::ColourMap](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/monovar.png?raw=true)

## HSV1D

`setHue()`

![HSV colour maps
 morph::ColourMap](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/hsv1d.png?raw=true)

# Static colour conversion methods

ColourMap contains some static methods that could be used in isolation.

```c++
// Convert float ([0, 1]) into a colour from the Jet colour map
static std::array<float,3> jetcolour (float datum);

// The Hue-Saturation-Value to Red-Green-Blue conversion
static std::array<float, 3> hsv2rgb (const std::array<folat, 3>& hsv);
static std::array<float, 3> hsv2rgb (float h, float s, float v);
static morph::vec<float, 3> hsv2rgb_vec (const morph::vec<float, 3>& hsv);

// RGB to HSV
static std::array<float, 3> rgb2hsv (float r, float g, float b);
```