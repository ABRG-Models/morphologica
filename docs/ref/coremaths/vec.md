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

The most up to date reference is always the header file [morph/vec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vec.h).

## Summary

`morph::vec` is a fixed-size mathematical vector class. Derives from
`std::array` and can be used in much the same way as its STL parent.

```c++
namespace morph {
    template <typename S=float, size_t N=3>
    struct vec : public std::array<S, N>
    {
```

Template arguments are `S`, the element type and `N`, the size of the array.

Create objects just like `std::array`:

```c++
morph::vec<int, 4> v1 = { 1, 2, 3, 4 };
```
but with `morph::vec`, you can do maths:

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

## Functions

### Setter functions
```c++
template <typename _S=S> // S and N from morph::vec class template
void set_from (const std::vector<_S>& vec);
void set_from (const std::array<_S, N>& ar);
void set_from (const std::array<_S, (N+1)>& ar);
void set_from (const std::array<_S, (N-1)>& ar);
void set_from (const vec<_S, (N-1)>& v);
```

These `set_from` functions set the values of this `morph::vec` from
other containers. Note that there are versions which take one-bigger
and one-smaller arrays; these are used in the graphics code where 3D
vectors are converted to 4D before being multiplied by 4x4 transform
matrices.

---

```c++
void set_from (const _S& v);
```
This `set_from` overload fills all elements of the `morph::vec` with `v`.

### Numpy clones

```c++
void linspace (const _S start, const _S2 stop);
void arange (const _S start, const _S2 stop, const _S2 increment);
```

Python Numpy-like functions to fill the `morph::vec` with sequences of
numbers.  `linspace` fills the `vec` with `N` values in a sequence
from `start` to `stop`. `arange` fills up to `N` elements starting
with `start` and ending with `stop` incrementing by `increment`. If
the function gets to the end of the array, then it simply stops. If it
fails to fill the array, remaining values will be 0.

### Plus-one/Less-one dimension

```c++
vec<S, N-1> less_one_dim () const;
vec<S, N+1> plus_one_dim () const;
vec<S, N+1> plus_one_dim (const S val) const;
```
Returns `morph::vec`s with one additional or one less element.

### Type conversions

```c++
vec<float, N> as_float() const;
vec<double, N> as_double() const;
vec<int, N> as_int() const;
vec<unsigned int, N> as_uint() const;
```
When you need to convert a `vec` of `float` into a `vec` of `int` and similar.

### String output

```c++
std::string str() const;
std::string str_mat() const;
std::string str_numpy() const;
```
These functions output the array as a string in different formats. The _mat and _numpy versions generate text that can be pasted into a session of MATLAB/Octave or Python.

### Rescale/renormalize

These functions use a template to ensure they are only enabled for non-integral types:
```c++
template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
```

```c++
void renormalize();
void rescale();
void rescale_neg();
void rescale_sym();
```











## Using morph::vec as a key in std::map or within an std::set

Although morph::vec derives from std::array, you **can't use it as a key in an std::map**.

```c++
// Bad!
std::map<morph::vec<int, 2>, myclass> some_map;

// Also Bad!
std::set<morph::vec<int, 2>> some_set;
```

The reason for this is that the less-than operation is redefined, but `std::set` and `std::map` depend upon less-than for their functionality. In std::array, less-than is lexicographic. See this file for a workaround, in which you specify that you want to use morph::vec's lexicographics less-than: [tests/testvec_asmapkey](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec_asmapkey.cpp).
