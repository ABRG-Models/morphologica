---
layout: page
title: morph::vec
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/vec/
nav_order: 1
---
## morph::vec (fixed-size mathematical vector)

Header file: [morph/vec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vec.h)
```c++
#include <morph/vec.h>
```

## Summary

`morph::vec` is a fixed-size mathematical vector class. It derives from
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
morph::vec<int, 4> v3 = v1 + v2;             // element-wise addition
morph::vec<float, 3> u1 = { 1.0f, 0.0f, 0.0f };
morph::vec<float, 3> u2 = { 0.0f, 1.0f, 0.0f };
morph::vec<float, 3> u3 = u1.cross (u2);     // vector cross-product
float dp = u1.dot (u3);                      // (scalar/dot/inner)-product
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

```c++
void set_from (const _S& v);
void zero();
void set_max();
void set_lowest();
```
This `set_from` overload fills all elements of the `morph::vec` with `v`. `zero()`, `set_max()` and `set_lowest()` fill all elements with `S{0}`, the maximum possible value for the type and the lowest possible value, respectively.

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

### Random numbers

Three functions to fill a `vec` with random numbers:
```c++
void randomize();                // fill from uniform random number generator, range [0,1).
void randomize (S min, S max)    // fill from uniform RNG, range [min,max).
void randomizeN (S _mean, S _sd) // fill from normal RNG with given mean and std. deviation.
```

### Plus-one/less-one dimension

```c++
vec<S, N-1> less_one_dim () const;
vec<S, N+1> plus_one_dim () const;
vec<S, N+1> plus_one_dim (const S val) const;
```
Returns a `morph::vec` with one additional or one less element.

### Type conversions

```c++
vec<float, N> as_float() const;
vec<double, N> as_double() const;
vec<int, N> as_int() const;
vec<unsigned int, N> as_uint() const;
```
When you need to convert a `vec` of `float` into a `vec` of `int` and similar:

```c++
morph::vec<int, 3> vi = {1,2,3};
morph::vec<float, 3> vf = vi.as_float(); // Note: new memory is allocated for the new object

```

### String output

```c++
std::string str() const;
std::string str_mat() const;
std::string str_numpy() const;
```
These functions output the array as a string in different formats. The _mat and _numpy versions generate text that can be pasted into a session of MATLAB/Octave or Python. Output looks like `(1,2,3)` (`str()`), `[1,2,3]` (`str_mat()`) or `np.array((1,2,3))` (`str_numpy()`). If you stream a `vec` then `str()` is used:
```c++
morph::vec<int, 3> v = {1,2,3}; // Make a vec called v
std::cout << v;                 // Stream to stdout
```
gives output `(1,2,3)`.

### Length, lengthen, shorten

```c++
template <typename _S=S>
_S length() const;                     // return the vector length
_S length_sq() const;                  // return the vector length squared
_S sos() const;                        // also length squared. See header for difference
// Enabled only for non-integral S:
vec<S, N> shorten (const S dl) const;  // return a vector shortened by length dl
vec<S, N> lengthen (const S dl) const; // return a vector lengthened by length dl
```

### Rescale/renormalize

```c++
void renormalize();  // make vector length 1
void rescale();      // rescale to range [0,1]
void rescale_neg();  // rescale to range [-1,0]
void rescale_sym();  // rescale to range [-1,1]
```
Renormalization takes a vector and makes it have length 1. Thus,
`renormalize()` computes the length of the `vec` and divides each element by this length.

Use `rescale` when you want the values in the array to range between 0
and 1. `recale` will find a linear scaling so that the values in the
`vec` will be in the range [0,1]. `rescale_neg` finds a scaling that
puts the values in the range [-1,0]. `rescale_sym` puts the values in
the range [-1,1].

Note that these functions use a template to ensure they are only enabled for non-integral types:
```c++
template <typename _S=S, std::enable_if_t<!std::is_integral<std::decay_t<_S>>::value, int> = 0 >
```

Check whether your renormalized vector is a unit vector:

```c++
bool checkunit() const; // return true if length is 1 (to within vec::unitThresh = 0.001)
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
