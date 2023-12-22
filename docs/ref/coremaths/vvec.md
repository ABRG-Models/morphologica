---
layout: page
parent: Core maths classes
grand_parent: Reference
title: morph::vvec
permalink: /ref/coremaths/vvec/
nav_order: 2
---
# The variable vector class, morph::vvec

```c++
#include <morph/vvec.h>
```

Header: [morph/vvec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vvec.h)

`vvec` is a dynamically re-sizable array which derives from `std::vector`.
It extends `std::vector` by providing maths methods.
It's a convenient and simple linear maths library, all in a single header.
The elements of a `vvec` take the template type for the class, which is commonly a scalar such as `int`, `float` or `double`.
However, it is also possible to create a `vvec` of vectors or coordinates.

Here's how the class is declared:

```c++
    template <typename S=float, typename Al=std::allocator<S>>
    struct vvec : public std::vector<S, Al>
    {
        //! We inherit std::vector's constructors like this:
        using std::vector<S, Al>::vector;
```
