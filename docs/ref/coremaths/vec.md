---
layout: page
title: morph::vec
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/vec/
nav_order: 1
---
## morph::vec (fixed-size mathematical vector)

```c++
#include <morph/vec.h>
```
Header file: [morph/vec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vec.h). Test and example code:  [tests/testvec](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec.cpp), [tests/testvecElementOps](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvecElementOps.cpp).

**Table of contents**

* [Summary](/ref/coremaths/vec/#summary)
* [Design](/ref/coremaths/vec/#design)
* [Arithmetic operators](/ref/coremaths/vec/#arithmetic-operators)
* [Assignment operators](/ref/coremaths/vec/#assignment-operators)
* [Comparison operators](/ref/coremaths/vec/#comparison-operators)
* [Member functions](/ref/coremaths/vec/#member-functions)
  - [Setter functions](/ref/coremaths/vec/#setter-functions)
  - [Numpy-like functions](/ref/coremaths/vec/#numpy-clones)
  - [Random numbers](/ref/coremaths/vec/#random-numbers)
  - [Plus-one/less-one dimension](/ref/coremaths/vec/#plus-oneless-one-dimension)
  - [Type conversions](/ref/coremaths/vec/#type-conversions)
  - [String output](/ref/coremaths/vec/#string-output)
  - [Length/lengthen/shorten](/ref/coremaths/vec/#length-lengthen-shorten)
  - [Range and renormalization](/ref/coremaths/vec/#the-range-and-rescaling-or-renormalizing)
  - [Finding elements](/ref/coremaths/vec/#finding-elements)
  - [Simple statistics](/ref/coremaths/vec/#simple-statistics)
  - [Maths functions](/ref/coremaths/vec/#maths-functions)

## Summary

`morph::vec` is a fixed-size mathematical vector class. It derives
from `std::array` and can be used in much the same way as its STL
parent. It has iterators and you can apply STL algorithms.

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

## Design

This was the first class where I decided to derive from an STL
container. I was motivated by a need for a fixed size mathematical
vector to use in the computation of three dimensional models for the
visualization code. I wanted only the data stored in a fixed size
std::array, but I wanted to add mathematical operations with an
interface that would make coding with the class convenient, easy and
enjoyable.

Inheriting from an STL container is generally discouraged but it works
very well because I add no additional member data attributes to the
derived class, only methods. The resulting class is extremely
convenient to use.

`std::array` is an 'aggregate class' with no user-provided constructors,
and morph::vec does not add any of its own constructors. It is generally initialized with brace-initializer lists.
```c++
vec<float, 3> v = { 1.0f , 1.0f, 1.0f };
```

The template arguments are `S` and `N` which are type and size. These
are passed through to std::array. The argument name `S` hints 'scalar'
as the elements of a mathematical vector of `N` dimensions are scalar,
real numbers.

## Arithmetic operators

You can use arithmetic operators on `vec` objects with their operations being applied element wise. operations with other `vec` objects and with scalars are all supported and should work as expected.

| Operator | Scalar example | Element wise `vec` example |
| --- | --- | --- |
| + | `v2 = v1 + 2.0f;` *or* `v2 = 2.0f + v1;` | `v3 = v1 + v2;` |
| += | `v2 += 2.0f;` | `v2 += v1;` |
| - | `v2 = v1 - 5.0f;` *or* `v2 = 5.0f - v1;` | `v3 = v1 - v2;` |
| -= | `v2 -= 2.0f;` | `v2 -= v1;` |
| *  | `v2 = v1 * 10.0;` *or* `v2 =  10.0 * v1;` | `v3 = v1 * v2;` |
| *= | `v2 *= 10;` | `v2 *= v1;` |
| /  | `v2 = v1 / 10.0;`  *or* `v2 = 10.0 / v1;`| `v3 = v1 / v2;` |
| /= | `v2 /= 10;` | `v2 /= v1;` |
| - (unary negate) |   | `v2 = -v1;` |

## Assignment operators

The assignment operator `=` will work correctly to assign one `vec` to another. For example,
```c++
morph::vec<float, 3> v1 = { 1, 2, 3 };
morph::vec<float, 3> v2 = v1;           // Good, works fine
```
Because `std::array` is the base class of `vec` it is also possible to assign a `vec` **to** an `std::array`:
```c++
morph::vec<float, 3> v1 = { 1, 2, 3 };
std::array<float, 3> a1 = v1;           // Good, works fine
```

However, there are no templated assignment operators in `morph::vec` that make it
possible to assign **from** other types (I *did* try). For example, this will **not** compile:
```c++
std::array<float, 3> a1 = { 1, 2, 3 };
morph::vec<float, 3> v1 = a1;           // Bad, doesn't compile
```

## Comparison operators

The default comparison in `std::array` is a **lexicographic comparison**. This means that if the first element in a first `array` is less than the first element in a second `array`, then the first `array` is less than the second `array` regardless of the values in the remaining elements. This is not useful when the array is interpreted as a mathematical vector. In this case, an appropriate comparison is probably **vector magnitude comparison** (i.e comparing their lengths). Another useful comparison is **elementwise comparison** where one vector may be considered to be less than another if *each* of its elements is less than the corresonding other element. That is to say `v1` < `v2` if `v1[i]` < `v2[i]` for all `i`. Both vector magnitude and elementwise comparison can be applied to a `morph::vec` and a scalar.

In `morph::vec` I've implemented the following comparisons. Note that the default is not lexicographic comparison and that some comparisions could still be implemented.

| Comparison (`vec v1` with `vec v2`)   | Lexicographic      | Vector magnitude  | Elementwise |
| --- | --- | --- | --- |
| `v1` < `v2`  | `v1.lexical_lessthan (v2)` | `v1.length_lessthan (v2)` |  `v1 < v2`   |
| `v1` <= `v2`  | not implemented | `v1.length_lte (v2)` |  `v1 <= v2`   |
| `v1` > `v2`  | not implemented | `v1.length_gtrthan (v2)` |  `v1 > v2`   |
| `v1` >= `v2`  | not implemented | `v1.length_gte (v2)` |  `v1 >= v2`   |

| Comparison (`vec v ` with `scalar s`)    | Lexicographic      | Vector magnitude  | Elementwise |
| --- | --- | --- | --- |
| `v` < `s` | not implemented | not implemented |  `v < s` |
| `v` <= `s` | not implemented | not implemented |  `v <= s` |
| `v` > `s` | not implemented | not implemented |  `v > s` |
| `v` >= `s` | not implemented | not implemented |  `v >= s` |

| 'not' comparison | Implementation |
| --- | --- |
| unary not operator `!` | `operator!` implements length comparison. `!v1` is true if `v1.length() == 0`  |
| not operator `!=` | unimplemented |

### Using `morph::vec` as a key in `std::map` or within an `std::set`

Although `morph::vec` derives from `std::array`, you **can't use it as a key in an `std::map`**!

**Danger!** The following examples **will compile**  but will have **unexpected runtime results**:

```c++
// Bad!! Because the key is morph::vec<int, 2>
std::map<morph::vec<int, 2>, myclass> some_map;

// Also Bad! Again, the key is morph::vec<int, 2>
std::set<morph::vec<int, 2>> some_set;
```

The reason for this is that `std::set` and `std::map` depend upon the less-than comparison for their functionality and the default element-wise less-than in `morph::vec` is *elementwise* which will fail to sort an array. In `std::array`, less-than is lexicographic which guarantees a successful sort.

The workaround is to specify that you want to use the `morph::vec` lexicographical less-than function `lexical_lessthan` in your `map` or `set`:
```c++
// To make the map work, we have to tell it to use lexical_lessthan:
auto _cmp = [](morph::vec<int,2> a, morph::vec<int,2> b){return a.lexical_lessthan(b);};
std::map<morph::vec<int, 2>, std::string, decltype(_cmp)> themap(_cmp);
```
This example comes from [tests/testvec_asmapkey](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec_asmapkey.cpp).

## Member functions

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
These return a new `vec` in the requested type:
```c++
vec<float, N> as_float() const;
vec<double, N> as_double() const;
vec<int, N> as_int() const;
vec<unsigned int, N> as_uint() const;
```
For example:
```c++
morph::vec<int, 3> vi = {1,2,3};
morph::vec<float, 3> vf = vi.as_float(); // Note: new memory is used for the new object
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

### The range and rescaling or renormalizing

You can obtain the range of values in the `vec` with `vec::range` which returns a [morph::range](/ref/coremaths/range) object:
```c++
morph::range<S> range() const;
```
Example usage:
```c++
morph::vec<float, 3> v = { 1, 2, 3 };
morph::range<float> r = v.range();
std::cout << "vec max: " << r.max << " and min: " << r.min << std::endl;
```
To re-scale or renormalize the values in the `vec`:
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

### Finding elements

A group of functions to find the value or index of particular elements in the array.
```c++
S longest() const;          // return longest element (greatest magnitude)
size_t arglongest() const;  // return index of longest element
S shortest() const;         // return shortest element (closest to 0)
size_t argshortest() const; // return index of shortest element
S max() const;              // return max element
size_t argmax() const;      // return index of max element
S min() const;              // return min element
size_t argmin() const;      // return index of min element
```

#### Content tests (zero, inf, NaN)

```c++
bool has_zero() const;
bool has_inf() const;         // can only return true if type S has infinity
bool has_nan() const;         // can only return true if type S has NaN
bool has_nan_or_inf() const;  // can only return true if type S has infinity and/or NaN
```

#### Replacing NaN and inf values

To replace all not-a-numbers in a `vec` with another value use:
```c++
void replace_nan_with (const S replacement)
```
To replace NaNs *and* infinitities, it's:
```c++
void replace_nan_or_inf_with (const S replacement)
```

### Simple statistics

```c++
// These template functions are declared with a type _S:
template<typename _S=S>

_S mean() const;          // The arithmetic mean
_S variance() const;      // The variance
_S std() const;           // The standard deviation
_S sum() const;           // The sum of all elements
_S product() const;       // The product of the elements
```

### Maths functions

Raising elements to a **power**.
```c++
vec<S, N> pow (const S& p) const;          // raise all elements to the power p, returning result in new vec
void pow_inplace (const S& p);             // in-place version which operates on the existing data in *this
template<typename _S=S>
vec<S, N> pow (const vec<_S, N>& p) const; // Raise each element in *this to the power of the matching element in p
template<typename _S=S>
void pow_inplace (const vec<_S, N>& p);    // in-place version
```

The **signum function** is 1 if a value is >0; -1 if the value is <0 and 0 if the value is 0.
```c++
vec<S, N> signum() const;    // Return the result of the signum function in a new vec
void signum_inplace();       // in-place version
```

**Floor** computes the largest integer value not greater than an element value. This is applied to each element.
```c++
vec<S, N> floor() const;     // Return the result of the floor function in a new vec
void floor_inplace();        // in-place version
```

**Ceil** computes the least integer value not less than an element value. This is applied to each element.
```c++
vec<S, N> ceil() const;      // Return the result of the ceil function in a new vec
void ceil_inplace();         // in-place version
```

**Trunc** computes the nearest integer not greater in magnitude than element value. This is applied to each element.
```c++
vec<S, N> trunc() const;     // Return the result of the trunc function in a new vec
void trunc_inplace();        // in-place version
```

**Square root** or the **square**. These are convenience functions. For cube root, etc, use `pow()`.
```c++
vec<S, N> sqrt() const;   // the square root
void sqrt_inplace();
vec<S, N> sq() const;     // the square
void sq_inplace();
```
**Logarithms** and **exponential**
```c++
vec<S, N> log() const;     // element-wise natural log
void log_inplace();
vec<S, N> log10() const;   // log to base 10
void log10_inplace();
vec<S, N> exp() const;     // element-wise exp
void exp_inplace();
```
**absolute value**/**magnitude**
```c++
vec<S, N> abs() const;     // element-wise abs()
void abs_inplace();
```

The **scalar product** (also known as inner product or dot product) can be computed for two `vec` instances:
```c++
template<typename _S=S>
S dot (const vec<_S, N>& v) const
```

The **cross product** is defined here only for `N`=2 or `N`=3.

If `N` is 2, then v x w is defined to be v_x w_y - v_y w_x and for N=3, see your nearest vector maths textbook. The function signatures are
```c++
template <typename _S=S, size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
S cross (const vec<_S, _N>& w) const;
template <typename _S=S, size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
vec<S, _N> cross (const vec<_S, _N>& v) const;
```

Also defined only in two dimensions are **angle** functions. `angle()` returns the angle of the `vec`. It wraps `std::atan2(y, x)`. `set_angle()` sets the angle of a 2D `vec`, maintaining its length or setting it to 1 if it is a zero vector.

```c++
S angle() const;                  // Returns the angle of the 2D vec in radians
void set_angle (const _S _ang);   // Set a two dimensional angle in radians
```
