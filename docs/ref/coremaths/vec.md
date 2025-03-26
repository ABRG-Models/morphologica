---
layout: page
title: morph::vec
parent: Core maths classes
grand_parent: Reference
permalink: /ref/coremaths/vec/
nav_order: 2
---
## morph::vec (fixed-size mathematical vector)

```c++
#include <morph/vec.h>
```
Header file: [morph/vec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vec.h). Test and example code:  [tests/testvec](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec.cpp), [tests/testvecElementOps](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvecElementOps.cpp).

**Table of contents**

- TOC
{:toc}

## Summary

`morph::vec` is a fixed-size mathematical vector class. It derives
from `std::array` and can be used in much the same way as its STL
parent. It has iterators and you can apply STL algorithms. It is
constexpr-capable, meaning that it can be incorporated into constexpr
functions to do compile-time maths. The majority of its methods are
`nopexcept` (they do *not* throw exceptions).

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
visualization code. I wanted the data stored in a fixed size
`std::array`, but I wanted to add mathematical operations with an
interface that would make coding with the class convenient, easy and
enjoyable.

While I could have created a class with an `std::array data` member, I
thought simply adding functions to an extended `std::array` would work
very well. It is indeed, extremely convenient to use.

However, you may see this approach discouraged. As [explained by Roger
Pate](https://stackoverflow.com/questions/2034916/is-it-okay-to-inherit-implementation-from-stl-containers-rather-than-delegate)
the risk is deallocating through a pointer to the base class, with the
issue being that STL containers don't have virtual deconstructors. I
avoid this issue by adding *no additional member data attributes* to
`morph::vec`, *only methods*.

`std::array` is an 'aggregate class' with no user-provided constructors,
and `morph::vec` does not add any of its own constructors. It is generally initialized with brace-initializer lists.
```c++
vec<float, 3> v = { 1.0f, 1.0f, 1.0f };
```

The template arguments are `S` and `N` which are type and size. These
are passed through to `std::array`. The argument name `S` hints 'scalar'
as the elements of a mathematical vector of `N` dimensions are scalar,
real numbers.

## Arithmetic operators

You can use arithmetic operators on `vec` objects with their operations being applied element-wise. operations with other `vec` objects and with scalars are all supported and should work as expected.

| Operator | Scalar example | Element-Wise `vec` example |
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
In this case, you have to use `vec::set_from`:
```c++
std::array<float, 3> a1 = { 1, 2, 3 };
morph::vec<float, 3> v1;
v1.set_from (a1);
```

## Comparison operators

The default comparison in `std::array` is a **lexicographic comparison**. This means that if the first element in a first `array` is less than the first element in a second `array`, then the first `array` is less than the second `array` regardless of the values in the remaining elements. This is not useful when the array is interpreted as a mathematical vector. In this case, an appropriate comparison is probably **vector magnitude comparison** (i.e comparing their lengths). Another useful comparison is **element-wise comparison** where one vector may be considered to be less than another if *each* of its elements is less than the corresponding element in the other. That is to say `v1` < `v2` if `v1[i]` < `v2[i]` for all `i`. Both vector magnitude and element-wise comparison can be applied to a `morph::vec` and a scalar.

In `morph::vec` I've implemented the following comparisons. Note that the default is not lexicographic comparison and that some comparisons could still be implemented.

| Comparison (`vec v1` with `vec v2`)   | Lexicographic      | Vector magnitude  | Element-Wise |
| --- | --- | --- | --- |
| `v1` < `v2`  | `v1.lexical_lessthan (v2)` | `v1.length_lessthan (v2)` |  `v1 < v2`   |
| `v1` <= `v2`  | not implemented | `v1.length_lte (v2)` |  `v1 <= v2`   |
| `v1` > `v2`  | not implemented | `v1.length_gtrthan (v2)` |  `v1 > v2`   |
| `v1` >= `v2`  | not implemented | `v1.length_gte (v2)` |  `v1 >= v2`   |

| Comparison (`vec v ` with `scalar s`)    | Lexicographic      | Vector magnitude  | Element-Wise |
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

Although `morph::vec` derives from `std::array`, you **can't use morph::vec as a key in an `std::map`**!

**Danger!** The following examples **will compile**  but will have **unexpected runtime results**:

```c++
// Bad!! Because the key is morph::vec<int, 2>
std::map<morph::vec<int, 2>, myclass> some_map;

// Also Bad! Again, the key is morph::vec<int, 2>
std::set<morph::vec<int, 2>> some_set;
```

The reason for this is that `std::set` and `std::map` depend upon the less-than comparison for their functionality and the default less-than comparison in `morph::vec` is *element-wise* which will fail to sort an array. In `std::array`, less-than is *lexicographic* which guarantees a successful sort.

If you really need to use `morph::vec` as a key, there *is* a workaround. You have to specify that you want to use the `morph::vec` lexicographical less-than function `lexical_lessthan` in your `map` or `set` like this:
```c++
// To make the map work, we have to create a comparison function that uses lexical_lessthan...
auto _cmp = [](morph::vec<int,2> a, morph::vec<int,2> b){return a.lexical_lessthan(b);};
// ...then pass this function when declaring the `map` (or `set`):
std::map<morph::vec<int, 2>, std::string, decltype(_cmp)> themap(_cmp);
```
This example comes from [tests/testvec_asmapkey](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec_asmapkey.cpp).

## Casting of `std::array` to `morph::vec`

If you have data in a part of your program contained in an `std::array`, it's possible to cast it to `morph::vec` either to use it within a morphologica visualization or simply to do some maths on the data.

The following casts from `array` to `vec` are possible.

### Cast `array` to a const pointer

Use `static_cast` or `reinterpret_cast` to cast an `array` to a `const vec` pointer.
```c++
// An array to cast
std::array<float, 3> a1 = { 3, 2, 1 };
// A function taking a const pointer argument
void f_const_ptr_v (const morph::vec<float, 3>* v1) { /* morph::vec operations */ }
// Calls to the function, passing in the array
f_const_ptr_v (static_cast< const morph::vec<float, 3>* >(&a1));
f_const_ptr_v (reinterpret_cast< const morph::vec<float, 3>* >(&a1));
f_const_ptr_v (static_cast< morph::vec<float, 3>* >(&a1));
f_const_ptr_v (reinterpret_cast< morph::vec<float, 3>* >(&a1));
```

### Cast `array` to a non-const pointer

You can only use `static_cast` to cast an `array` to a non-const pointer.
```c++
void f_nonconst_ptr_v (morph::vec<float, 3>* v1) { /* morph::vec operations */ }
f_nonconst_ptr_v (static_cast< morph::vec<float, 3>* >(&a1));
```

### Cast `array` to a const reference

You can use `static_cast` or `reinterpret_cast` to cast `std::array` to a `const vec<>&`.
```c++
void f_const_ref_v (const morph::vec<float, 3>& v1) { /* morph::vec operations */ }
f_const_ref_v (static_cast< const morph::vec<float, 3>& >(a1));
f_const_ref_v (reinterpret_cast< const morph::vec<float, 3>& >(a1));
f_const_ref_v (static_cast< morph::vec<float, 3>& >(a1));
f_const_ref_v (reinterpret_cast< morph::vec<float, 3>& >(a1));
```

### Cast `array` to a non-const reference

You can only use `static_cast` to cast `array` to a non-const `vec` reference.
```c++
void f_nonconst_ref_v (morph::vec<float, 3>& v1) { /* morph::vec operations */ }
f_nonconst_ref_v (static_cast< morph::vec<float, 3>& >(a1));
```

## Casting `morph::vec` to `std::array`

You may need to use a third-party library to process data in a `morph::vec`. You can use these casts to avoid any need to duplicate the data.

### Cast to a const `array` pointer
You can use static, reinterpret or dynamic casts to cast from `morph::vec` to `std::array`. Declaring that the pointer must be const is optional.
```c++
morph::vec<float, 3> v1 = { 1, 2, 3 };
void f_const_ptr_a (const std::array<float, 3>* a1) { /* std::array operations */ }
f_const_ptr_a (static_cast<std::array<float, 3>*>(&v1));
f_const_ptr_a (reinterpret_cast<std::array<float, 3>*>(&v1));
f_const_ptr_a (dynamic_cast<std::array<float, 3>*>(&v1));
f_const_ptr_a (static_cast<const std::array<float, 3>*>(&v1));
f_const_ptr_a (reinterpret_cast<const std::array<float, 3>*>(&v1));
f_const_ptr_a (dynamic_cast<const std::array<float, 3>*>(&v1));
```
### Cast to a non-const `array` pointer
You can static cast or reinterpret_cast to cast `vec` to a non-const `array` pointer
```c++
void f_nonconst_ptr_a (std::array<float, 3>* a1) { /* std::array operations */ }
f_nonconst_ptr_a (static_cast<std::array<float, 3>*>(&v1));
f_nonconst_ptr_a (reinterpret_cast<std::array<float, 3>*>(&v1));
```
### Cast to a const `array` reference
Two possibilities for `static_cast` and one for `reinterpret_cast`.
```c++
void f_const_ref_a (const std::array<float, 3>& a1) { /* std::array operations */ }
f_const_ref_a (static_cast< std::array<float, 3> >(v1));
f_const_ref_a (static_cast< std::array<float, 3>& >(v1));
f_const_ref_a (dynamic_cast< std::array<float, 3>& >(v1));
```

### Cast to a non-const `array` reference
You can either static_cast or dynamic_cast from `vec` to `array` by reference.
```c++
void f_nonconst_ref_a (std::array<float, 3>& a1) { /* std::array operations */ }
f_nonconst_ref_a (static_cast< std::array<float, 3>& >(v1));
f_nonconst_ref_a (dynamic_cast< std::array<float, 3>& >(v1));
```

## Member functions

### Setter functions
```c++
template <typename Sy=S> // S and N from morph::vec class template
void set_from (const std::vector<Sy>& vec);
void set_from (const std::array<Sy, N>& ar);
void set_from (const std::array<Sy, (N+1)>& ar);
void set_from (const std::array<Sy, (N-1)>& ar);
void set_from (const vec<Sy, (N-1)>& v);
```

These `set_from` functions set the values of this `morph::vec` from
other containers. Note that there are versions which take one-bigger
and one-smaller arrays; these are used in the graphics code where 3D
vectors are converted to 4D before being multiplied by 4x4 transform
matrices.

```c++
void set_from (const Sy& v);
void zero();
void set_max();
void set_lowest();
```
This `set_from` overload fills all elements of the `morph::vec` with `v`. `zero()`, `set_max()` and `set_lowest()` fill all elements with `S{0}`, the maximum possible value for the type and the lowest possible value, respectively.

### Numpy clones

```c++
void linspace (const Sy start, const Sy2 stop);
void arange (const Sy start, const Sy2 stop, const Sy2 increment);
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

### Get first and last elements in the vec

Get first (0th) and last (N-1 th) elements in the vec. If vec is of zero size, returns a 2 element vec containing zeros.
```c++
morph::vec<int, 3> vi3 = { 1, 2, 3 };
morph::vec<int, 2> fl3 = vi3.firstlast();
std::cout << fl3; // (1, 3)

morph::vec<int, 2> vi2 = { 1, 2 };
morph::vec<int, 2> fl2 = vi2.firstlast();
std::cout << fl2; // (1, 2)

morph::vec<int, 1> vi1 = { 2 };
morph::vec<int, 2> fl1 = vi1.firstlast();
std::cout << fl1; // (2, 2)
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
template <typename Sy=S>
Sy length() const;                     // return the vector length
Sy length_sq() const;                  // return the vector length squared
Sy sos() const;                        // also length squared. See header for difference
// Enabled only for non-integral S:
vec<S, N> shorten (const S dl) const;  // return a vector shortened by length dl
vec<S, N> lengthen (const S dl) const; // return a vector lengthened by length dl
```

### The range and rescaling or renormalizing

You can obtain the range of values in the `vec` with `vec::range` which returns a [morph::range](/morphologica/ref/coremaths/range) object:
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
template <typename Sy=S, std::enable_if_t<!std::is_integral<std::decay_t<Sy>>::value, int> = 0 >
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
// These template functions are declared with a type Sy:
template<typename Sy=S>

Sy mean() const;          // The arithmetic mean
Sy variance() const;      // The variance
Sy std() const;           // The standard deviation
Sy sum() const;           // The sum of all elements
Sy product() const;       // The product of the elements
```

### Maths functions

Raising elements to a **power**.
```c++
vec<S, N> pow (const S& p) const;          // raise all elements to the power p, returning result in new vec
void pow_inplace (const S& p);             // in-place version which operates on the existing data in *this
template<typename Sy=S>
vec<S, N> pow (const vec<Sy, N>& p) const; // Raise each element in *this to the power of the matching element in p
template<typename Sy=S>
void pow_inplace (const vec<Sy, N>& p);    // in-place version
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
template<typename Sy=S>
S dot (const vec<Sy, N>& v) const
```

The **cross product** is defined here only for `N`=2 or `N`=3.

If `N` is 2, then v x w is defined to be v_x w_y - v_y w_x and for N=3, see your nearest vector maths textbook. The function signatures are
```c++
template <typename Sy=S, size_t _N = N, std::enable_if_t<(_N==2), int> = 0>
S cross (const vec<Sy, _N>& w) const;
template <typename Sy=S, size_t _N = N, std::enable_if_t<(_N==3), int> = 0>
vec<S, _N> cross (const vec<Sy, _N>& v) const;
```

Also defined only in two dimensions are **angle** functions. `angle()` returns the angle of the `vec`. It wraps `std::atan2(y, x)`. `set_angle()` sets the angle of a 2D `vec`, maintaining its length or setting it to 1 if it is a zero vector.

```c++
S angle() const;                  // Returns the angle of the 2D vec in radians
void set_angle (const Sy _ang);   // Set a two dimensional angle in radians
```
