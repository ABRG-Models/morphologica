---
layout: page
title: morph::vec
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/vec/
nav_order: 1
---
```c++
#include <morph/vec.h>
```

A fixed-size mathematical vector class. Derives from `std::array`.

Create like an `std::array`:

```c++
morph::vec<int, 4> v1 = { 1, 2, 3, 4 };
```
but you can do maths:

```c++
morph::vec<int, 4> v2 = { 1, 2, 3, 4 };
morph::vec<int, 4> v3 = v1 + v2; // element-wise addition
```

It's really useful to be able to do vector maths:

```c++
morph::vec<float, 3> u1 = { 1.0f, 0.0f, 0.0f };
morph::vec<float, 3> u2 = { 0.0f, 1.0f, 0.0f };
morph::vec<float, 3> u3 = u1.cross (u2);
morph::vec<float, 3> u4 = u1 - u2;
float dp = u1.dot (u3);
```

See these programs for more example usage: [tests/testvec](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec.cpp), [tests/testvecElementOps](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvecElementOps.cpp).

## Using morph::vec as a key in std::map or within an std::set

Although morph::vec derives from std::array, you **can't use it as a key in an std::map**.

```c++
// Bad!
std::map<morph::vec<int, 2>, myclass> some_map;

// Also Bad!
std::set<morph::vec<int, 2>> some_set;
```

The reason for this is that the less-than operation is redefined, but `std::set` and `std::map` depend upon less-than for their functionality. In std::array, less-than is lexicographic. See this file for a workaround, in which you specify that you want to use morph::vec's lexicographics less-than: [tests/testvec_asmapkey](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec_asmapkey.cpp).
