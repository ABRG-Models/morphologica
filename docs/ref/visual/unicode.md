---
title: morph::unicode
parent: Visualization API
grand_parent: Reference
permalink: /ref/visual/unicode
layout: page
nav_order: 14
---
```c++
#include <morph/unicode.h>
```

# Unicode text handling

`morph::unicode` is a class that provides a set of named unicode character codes along with a function to generate valid UTF-8 character sequences. It helps morphologica to display non-Ascii characters like the Greek alpha and beta characters shown in this graph (the superscripts are also unicode characters):

![A graph example in which the unicode characters for Greek alpha and beta are shown](https://github.com/ABRG-Models/morphologica/blob/main/docs/images/twinax.png?raw=true)

## Generating UTF-8 for a Unicode character

Usually, you just use the static function `unicode::toUtf8` which returns a `std::string` containing the UTF-8 sequence for the passed-in unicode character:

```c++
using morph::unicode; // To keep lines short
std::string alpha_character = unicode::toUtf8 (unicode::alpha);
```

You can output this string to `stdout` on a modern terminal and it will render the character.

You can append the string to another string:

```c++
std::string desc = "The Greek alpha character is: ";
desc += alpha_character;
```

You can pass either of these to `VisualModel::addLabel` as long as you use the Deja Vu font. The Deja Vu fonts contain a wide range of character glyphs for most of the useful unicode characters. Deja Vu Sans (`VisualFont::DVSans`) is the default font:

```c++
vm_ptr->addLabel (alpha_character, {0, -1, 0}, morph::TextFeatures(0.06f));
vm_ptr->addLabel (desc, {0, -1.2, 0}, morph::TextFeatures(0.06f));
```

You can explicitly choose one of the four Deja Vu font options, but note that the Vera fonts do not have many unicode character glyphs.

```c++
// Deja Vu Sans
vm_ptr->addLabel (desc, {0,-1,0}, morph::TextFeatures (0.06f, // font size
                                                       48,    // font res
                                                       false, // don't centre horizontally
                                                       morph::colour::black,
                                                       morph::VisualFont::DVSans));
// Deja Vu Sans Italic
vm_ptr->addLabel (desc, {0,-1,0}, morph::TextFeatures (0.06f, 48, false, morph::colour::black,
                                                       morph::VisualFont::DVSansItalic));
// Deja Vu Sans Bold
vm_ptr->addLabel (desc, {0,-1,0}, morph::TextFeatures (0.06f, 48, false, morph::colour::black,
                                                       morph::VisualFont::DVSansBold));
// Deja Vu Sans Bold Italic
vm_ptr->addLabel (desc, {0,-1,0}, morph::TextFeatures (0.06f, 48, false, morph::colour::black,
                                                       morph::VisualFont::DVSansBoldItalic));
```

## Named Unicode characters

The names for the character constants in morph::unicode have been chosen to be as intuitive as possile. The often-used Greek characters are `unicode::alpha`, `unicode::beta`, `unicode::gamma` and so on, with upper case versions as `unicode::Alpha`;,`unicode::Beta` etc. There is a wide range of mathematical symbols, arrow symbols, sub- and super-scripts. You can consult [unicode.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/unicode.h) for the full list.

## Raw Unicode

You don't have to pass unicodes by the `morph` namespace name; you can simply pass the code itself. For example, the unicode for the multiplication sign is 0x00d7, so these lines are equivalent:

```c++
using morph::unicode;
std::string mult1 = unicode::toUtf8 (unicode::multiplies);
std::string mult2 = unicode::toUtf8 (0x00d7);
```

## Additional helper functions

### Append

Append a unicode character to a string with `unicode::append`:

```c++
using morph::unicode;
std::string desc = "This is a Greek delta: ";
unicode::append (desc, unicode::delta);
```

### Subscripts and superscripts

Get a subscript:

```c++
std::string zeroth_x = "x" + unicode::subs(0);
```

Get a superscript:

```c++
std::string x_squared = "x" + unicode::ss(2);
```

## Conversion from UTF-8 back to Unicode

You'll find the function `morph::unicode::fromUtf8()` in unicode.h; this function is generally used only internally within morphologica. The function declaration is

```c++
static std::basic_string<char32_t> fromUtf8 (const std::string& input);
```