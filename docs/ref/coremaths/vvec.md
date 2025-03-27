---
layout: page
parent: Core maths classes
grand_parent: Reference
title: morph::vvec
permalink: /ref/coremaths/vvec/
nav_order: 3
---
## morph::vvec  (dynamically resizable mathematical vector)

```c++
#include <morph/vvec.h>
```
Header file: [morph/vvec.h](https://github.com/ABRG-Models/morphologica/blob/main/morph/vvec.h). Test and example code:  [tests/testvvec](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvvec.cpp)

**Table of contents**

- TOC
{:toc}

## Summary

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

Template arguments are `S`, the element type and `N`, the size of the array.

Create objects just like `std::vector`:

```c++
morph::vvec<int> v1 = { 1, 2, 3, 4 };
```
but with `morph::vvec`, and just like `morph::vec`, you can do maths:

```c++
morph::vvec<int> v2 = { 1, 2, 3, 4 };
morph::vvec<int> v3 = v1 + v2;                // element-wise addition
morph::vvec<float> u1 = { 1.0f, 0.0f, 0.0f };
morph::vvec<float> u2 = { 0.0f, 1.0f, 0.0f };
morph::vvec<float> u3 = u1.cross (u2);        // vector cross-product
float dp = u1.dot (u3);                       // (scalar/dot/inner)-product
```

## Design

Like `morph::vec` , `vvec` derives from an STL container without
apology to give us a dynamically resizable container of scalars or
vectors on which mathematical operations can be called.

`vvec` is very similar to `vec`, sharing many member functions with the same name.

Some methods may throw exceptions, those that do not are marked `noexcept`.

## Access

As an `std::vector`-like object, your `vvec` is indexed just like your `vector`. Use any of the array access `operator[]`, the `at()` method, or STL iterators.

```c++
morph::vvec<int> vvf = { 1, 2, 3 };
std::cout << "First element of array: " << vvf[0] << std::endl;
try {
  std::cout << "First element of array: " << vvf.at(0) << std::endl;
} catch const (const std::out_of_range& e) { /* Uh oh */ }
morph::vvec<int>::iterator vvf_iter = vvf.begin();
std::cout << "First element of array: " << *vvf_iter << std::endl;
```

### Signed indices

`vvec` does introduce one new way to index its content.
This is the ability to use a signed index with the methods `at_signed()` and `c_at_signed()`.
This allows you to access elements at the end or your `vvec` with this code:

```c++
morph::vvec<int> vvf = { 1, 2, 3 };
try {
  std::cout << "This will output '3'" << vvf.at_signed (-1) << std::endl;
  std::cout << "This will output '2'" << vvf.at_signed (-2) << std::endl;
  std::cout << "This will output '1'" << vvf.at_signed (-3) << std::endl;
  std::cout << "This will cause an exception: " << vvf.at_signed (-4) << std::endl;
} catch const (const std::out_of_range& e) { /* Too negative (or too positive) */ }
```

Non-negative indices work as if you called `.at(index)`:
```c++
try {
  std::cout << "This will output '1'" << vvf.at_signed (0) << std::endl;
  std::cout << "This will output '2'" << vvf.at_signed (1) << std::endl;
  std::cout << "This will output '3'" << vvf.at_signed (2) << std::endl;
  std::cout << "This will cause an exception: " << vvf.at_signed (3) << std::endl;
} catch const (const std::out_of_range& e) { /* Too positive (or too negative) */ }
```

`at_signed()` was added to access a quantity that was naturally indexed with a [-m +m] range of indices (the order of a spherical harmonic function).
Internally, it uses `.at()` and iterators.
It is a templated function that is only enabled for signed index type (`at_signed(1u)` will not compile)).


`c_at_signed()` is the `const` version of `at_signed()` for situations where you need to read from your `vvec` with a promise not to change any of the elements.

## Arithmetic operators

You can use arithmetic operators on `vvec` objects with their operations being applied element wise. operations with other `vvec` objects and with scalars are all supported and should work as expected.

| Operator | Scalar example | Element wise `vvec` example |
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

The assignment operator `=` will work correctly to assign one `vvec` to another. For example,
```c++
morph::vvec<float> v1 = { 1, 2, 3 };
morph::vvec<float> v2 = v1;            // Good, works fine
```
Because `std::vector` is the base class of `vvec` it is also possible to assign a `vvec` **to** an `std::vector`:
```c++
morph::vvec<float> v1 = { 1, 2, 3 };
std::vector<float> sv1 = v1;           // Good, works fine
```

However, there are no templated assignment operators in `morph::vvec` that make it
possible to assign **from** other types. For example, this will **not** compile:
```c++
std::vector<float> sv1 = { 1, 2, 3 };
morph::vvec<float> v1 = sv1;            // Bad, doesn't compile
```

## Comparison operators

The default comparison in `std::vector` (and also in `std::array`) is a **lexicographic comparison**. This means that if the first element in a first `vector` is less than the first element in a second `vector`, then the first `vector` is less than the second `vector` regardless of the values in the remaining elements. This is not useful when the vector is interpreted as a mathematical vector. In this case, an appropriate comparison is probably **vector magnitude comparison** (i.e comparing their lengths). Another useful comparison is **elementwise comparison** where one vector may be considered to be less than another if *each* of its elements is less than the corresonding other element. That is to say `v1` < `v2` if `v1[i]` < `v2[i]` for all `i`. Both vector magnitude and elementwise comparison can be applied to a `morph::vvec` and a scalar.

In `morph::vvec` I've implemented the following comparisons. Note that the default is not lexicographic comparison and that some comparisions could still be implemented.

| Comparison (`vvec v1` with `vvec v2`)   | Lexicographic      | Vector magnitude  | Elementwise |
| --- | --- | --- | --- |
| `v1` < `v2`  | `v1.lexical_lessthan (v2)` | `v1.length_lessthan (v2)` |  `v1 < v2`   |
| `v1` <= `v2`  | not implemented | `v1.length_lte (v2)` |  `v1 <= v2`   |
| `v1` > `v2`  | not implemented | `v1.length_gtrthan (v2)` |  `v1 > v2`   |
| `v1` >= `v2`  | not implemented | `v1.length_gte (v2)` |  `v1 >= v2`   |

| Comparison (`vvec v ` with `scalar s`)    | Lexicographic      | Vector magnitude  | Elementwise |
| --- | --- | --- | --- |
| `v` < `s` | not implemented | not implemented |  `v < s` |
| `v` <= `s` | not implemented | not implemented |  `v <= s` |
| `v` > `s` | not implemented | not implemented |  `v > s` |
| `v` >= `s` | not implemented | not implemented |  `v >= s` |

| 'not' comparison | Implementation |
| --- | --- |
| unary not operator `!` | `operator!` implements length comparison. `!v1` is true if `v1.length() == 0`  |
| not operator `!=` | unimplemented |

### Using `morph::vvec` as a key in `std::map` or within an `std::set`

Although `morph::vvec` derives from `std::vector`, you **can't use it as a key in an `std::map`**!

**Danger!** The following examples **will compile**  but will have **unexpected runtime results**:

```c++
// Bad!! Because the key is morph::vvec<int>
std::map<morph::vvec<int>, myclass> some_map;

// Also Bad! Again, the key is morph::vvec<int>
std::set<morph::vvec<int>> some_set;
```

The reason for this is that `std::set` and `std::map` depend upon the less-than comparison for their functionality and the default element-wise less-than in `morph::vvec` is *elementwise* which will fail to sort an array. In `std::vector`, less-than is lexicographic which guarantees a successful sort.

The workaround is to specify that you want to use the `morph::vvec` lexicographical less-than function `lexical_lessthan` in your `map` or `set`:
```c++
// To make the map work, we have to tell it to use lexical_lessthan:
auto _cmp = [](morph::vvec<int> a, morph::vvec<int> b){return a.lexical_lessthan(b);};
std::map<morph::vvec<int>, std::string, decltype(_cmp)> themap(_cmp);
```
This example is adapted from [tests/testvec_asmapkey](https://github.com/ABRG-Models/morphologica/blob/main/tests/testvec_asmapkey.cpp) and may need to be tested!

## Member functions

### Setter functions

Rather than using implementations of the assignment operators (I did try, but it turned out to be too difficult), `vvec` provides `set_from` functions.
There is an overload of `set_from` which can assign values to your `vvec` from another container of values and an overload which will assign a single value to every element of your `vvec`.

Note that the `set_from` functions in `vvec` differ from those in `morph::vec`.

#### Setting a vvec from another container of values

If you want to set your vvec values from another container, such as a `std::vector`, then you can use this templated overload:

```c++
template <typename Container>
std::enable_if_t<morph::is_copyable_container<Container>::value
                 && !std::is_same<std::decay_t<Container>, S>::value, void>
set_from (const Container& c)
{
    this->resize (c.size());
    std::copy (c.begin(), c.end(), this->begin());
}
```

As long as you pass in a 'copyable container' (which means a `vector`, `array`, `deque`, `vvec`, `vec` or `set`, but *not* a `map`) then this will resize your `vvec` to match the passed in container and then std::copy the values into your `vvec`. Note that this is *not* enabled if the copyable container type is the same as the element type 'S' of the `vvec`, which may occur if you have a '`vvec` of `vec`s' and you `set_from` a `vec`. In that case, you would set all elements of the `vvec` to the passed in `vec`.

#### Setting all the elements of a vvec to the same value

All of these functions set all the elements of your `vvec` to a single value.

```c++
void set_from (const Sy& v);
void zero();
void set_max();
void set_lowest();
```
The `set_from` overload fills all elements of the `morph::vvec` with `v`. For example:

```c++
morph::vvec<int> vi = { 1, 2, 3 };
vi.set_from (4);
std::cout << vi << std::endl; // output: (4, 4, 4)
```
This works even if you have a vvec of array types:
```c++
morph::vvec< morph::vec<float, 2> > vvecofvecs(2);
vvecofvecs.set_from (morph::vec<float, 2>{3, 4});
std::cout << vvecofvecs << std::endl; // output: ((3, 4), (3, 4))
```

`zero()`, `set_max()` and `set_lowest()` fill all elements with `S{0}`, the maximum possible value for the type and the lowest possible value, respectively.

#### Using the parent class's `assign` method

You can, of course, also use [the `assign()` method](https://en.cppreference.com/w/cpp/container/vector/assign) from `std::vector`:

```c++
morph::vvec<int> vv = { 0, 0, 0, 0, 0}; // construct with 5 elements
vv.assign (3, 10);                      // resize vv to 3 elements and assign 10 to each element
std::cout << vv << std::endl;           // output: (10,10,10)
```

or

```c++
std::vector<int> vin = { 1, 2, 3 };
morph::vvec<int> vv (vin.size());
vv.assign (vin.begin(), vin.end());
std::cout << vv << std::endl;           // output: (1,2,3)
```

### Numpy clones

```c++
void linspace (const Sy start, const Sy2 stop, const size_t num=0);
void arange (const Sy start, const Sy2 stop, const Sy2 increment);
```

Python Numpy-like functions to fill the `morph::vvec` with sequences
of numbers.  `linspace` fills the `vvec` with `num` values in a
sequence from `start` to `stop`. If `num` is 0, then the vvec's size
is not changed and it is filled with an evenly spaced sequence of
values from `start` to `stop`. This behaviour differs from Python,
which would return an empty array for `num = 0`. However, the result
of `linspace (start, stop, 1)` matches Python: a single-element `vvec`
containing only the start value.

`arange` resizes the `vvec` and fills it with elements starting
with `start` and ending with `stop` incrementing by `increment`.

### Random numbers

Three functions to fill a `vvec` with random numbers:
```c++
void randomize();                // fill from uniform random number generator, range [0,1).
void randomize (S min, S max)    // fill from uniform RNG, range [min,max).
void randomizeN (S _mean, S _sd) // fill from normal RNG with given mean and std. deviation.
```

### Re-ordering elements

There's a shuffle function:
```c++
void shuffle();                  // Randomize the order of the existing elements
vvec<S> shuffled();              // Return a vvec copy with randomized the order of the existing elements
```
And rotates:

```c++
void rotate();                                // Rotate '1 step to the left'
template <typename T=int> void rotate (T n);  // Rotates 'n steps to the left'
void rotate_pairs(); // If size is even, permute pairs of elements in a rotation. 0->1, 1->0, 2->3, 3->2, etc.
```

### Type conversions
These return a new `vvec` in the requested type:
```c++
vvec<float> as_float() const;
vvec<double> as_double() const;
vvec<int> as_int() const;
vvec<unsigned int> as_uint() const;
```
For example:
```c++
morph::vvec<int> vi = {1,2,3};
morph::vvec<float> vf = vi.as_float(); // Note: new memory is used for the new object
```
### Get first and last elements in the vvec

Get first (`0`th) and last (`size()-1` th) elements in the vvec. If vvec is of zero size, returns a 2 element vvec containing zeros.
```c++
morph::vvec<int> vv3 = { 1, 2, 3 };
morph::vvec<int> fl3 = vv3.firstlast();
std::cout << fl3; // (1, 3)

morph::vvec<int> vv2 = { 1, 2 };
morph::vvec<int> fl2 = vv2.firstlast();
std::cout << fl2; // (1, 2)

morph::vvec<int> vv1 = { 2 };
morph::vvec<int> fl1 = vv1.firstlast();
std::cout << fl1; // (2, 2)

morph::vvec<int> vv0 = {};
morph::vvec<int> fl0 = vv1.firstlast();
std::cout << fl0; // (0, 0)
```

### String output

```c++
std::string str() const;
std::string str_mat() const;
std::string str_numpy() const;
```
These functions output the `vvec` as a string in different formats. The _mat and _numpy versions generate text that can be pasted into a session of MATLAB/Octave or Python. Output looks like `(1,2,3)` (`str()`), `[1,2,3]` (`str_mat()`) or `np.array((1,2,3))` (`str_numpy()`). If you stream a `vvec` then `str()` is used:
```c++
morph::vvec<int> v = {1,2,3};   // Make a vvec called v
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
vvec<S> shorten (const S dl) const;    // return a vector shortened by length dl
vvec<S> lengthen (const S dl) const;   // return a vector lengthened by length dl
```

### The range and rescaling or renormalizing

You can obtain the range of values in the `vvec` with `vvec::range` which returns a [morph::range](/morphologica/ref/coremaths/range) object:
```c++
morph::range<S> range() const;
```
Example usage:
```c++
morph::vvec<float> v = { 1, 2, 3 };
morph::range<float> r = v.range();
std::cout << "vvec max: " << r.max << " and min: " << r.min << std::endl;
```
If the contained type is itself a vector, then `vvec::range()` returns the shortest vector as min and the longest as max.

```c++
morph::vvec<morph::vec<int, 2>> v = { {-1, -3},   {-2, 4},  {3, 5} };
morph::range<morph::vec<int, 2>> r = v.range();
std::cout << "r.min: " << r.min; // {-1, -3}
std::cout << "r.max: " << r.max; // {3, 5}
```

To re-scale or renormalize the values in the `vvec`:
```c++
void renormalize();  // make vector length 1
void rescale();      // rescale to range [0,1]
void rescale_neg();  // rescale to range [-1,0]
void rescale_sym();  // rescale to range [-1,1]
```
Renormalization takes a vector and makes it have length 1. Thus,
`renormalize()` computes the length of the `vvec` and divides each element by this length.

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
bool checkunit() const; // return true if length is 1 (to within vvec::unitThresh = 0.001)
```
### Extent

The 'extent' of a vvec of scalar values is the same as its range (and `vvec<S>::extent()` simply sub-calls `vvec<S>::range()` for scalar `S`).
However, for a vvec of vector values, the extent returns a range of two vectors which define a volume that will enclose all the vectors contained in the vvec.
The vectors values must be given in some fixed size type, such as `std::array<>` or `morph::vec<>` (otherwise the function will not compile).

```c++
morph::vvec<morph::vec<int, 2>> v = { {-1, -3},   {-2, 4},  {3, 5} };
morph::range<morph::vec<int, 2>> r = v.extent();
std::cout << "r.min: " << r.min; // {-2, -3}
std::cout << "r.max: " << r.max; // {3, 5}
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

To replace all not-a-numbers in a `vvec` with another value use:
```c++
void replace_nan_with (const S replacement);
```
To replace NaNs *and* infinitities, it's:
```c++
void replace_nan_or_inf_with (const S replacement);
```

#### Search/replace and thresholding

`vvec` has search/replace:
```c++
void search_replace (const S searchee, const S replacement);
```

And threshold functions which replace any values outside limits with the limit.
```c++
vvec<S> threshold (const S lower, const S upper) const;
void threshold_inplace (const S lower, const S upper);
```

#### Pruning - removing elements from a `vvec`

There are also 'pruning' functions to *remove* positive or negative or nan values in a `vvec`, thus changing the size of the vvec or returning a vvec of smaller size.

```c++
vvec<S> prune_positive() const;
void prune_positive_inplace();
vvec<S> prune_negative() const;
void prune_negative_inplace();
vvec<S> prune_nan() const;
void prune_nan_inplace();
```

### Simple statistics

These template functions are declared with a boolean that directs code to account for NaNs in the data and a type `Sy`:

```c++
// All are:
template<bool test_for_nans=false, typename Sy=S>

Sy mean() const;          // The arithmetic mean
Sy variance() const;      // The variance
Sy std() const;           // The standard deviation
Sy sum() const;           // The sum of all elements
Sy sos() const;           // The sum of the squared elements
Sy product() const;       // The product of the elements
```

That means that you can obtain the statistic in type `Sy`, ignoring NaN values with calls like:

```c++
using fl = std::numeric_limits<float>;
morph::vvec<float> nums = { 1.0f, 2.1f, fl::quietNaN(), 3.2f };
double themean = nums.mean<true, double>();
```

### Maths functions

Raising elements to a **power**.
```c++
vvec<S> pow (const S& p) const;          // raise all elements to the power p, returning result in new vec
void pow_inplace (const S& p);           // in-place version which operates on the existing data in *this
template<typename Sy=S>
vec<S, N> pow (const vvec<Sy>& p) const; // Raise each element in *this to the power of the matching element in p (which must be same size)
template<typename Sy=S>
void pow_inplace (const vvec<Sy>& p);    // in-place version
```

The **signum function** is 1 if a value is >0; -1 if the value is <0 and 0 if the value is 0.
```c++
vvec<S> signum() const;      // Return the result of the signum function in a new vec
void signum_inplace();       // in-place version
```

**Floor** computes the largest integer value not greater than an element value. This is applied to each element.
```c++
vvec<S> floor() const;       // Return the result of the floor function in a new vec
void floor_inplace();        // in-place version
```

**Ceil** computes the least integer value not less than an element value. This is applied to each element.
```c++
vvec<S> ceil() const;        // Return the result of the ceil function in a new vec
void ceil_inplace();         // in-place version
```

**Trunc** computes the nearest integer not greater in magnitude than element value. This is applied to each element.
```c++
vvec<S> trunc() const;       // Return the result of the trunc function in a new vec
void trunc_inplace();        // in-place version
```

**Square root** or the **square**. These are convenience functions. For cube root, etc, use `pow()`.
```c++
vvec<S> sqrt() const;   // the square root
void sqrt_inplace();
vvec<S> sq() const;     // the square
void sq_inplace();
```
**Logarithms** and **exponential**
```c++
vvec<S> log() const;     // element-wise natural log
void log_inplace();
vvec<S> log10() const;   // log to base 10
void log10_inplace();
vvec<S> exp() const;     // element-wise exp
void exp_inplace();
```
**absolute value**/**magnitude**
```c++
vvec<S> abs() const;     // element-wise abs()
void abs_inplace();
```

The **scalar product** (also known as inner product or dot product) can be computed for two `vvec` instances, which must have equal size (otherwise a runtime error is thrown).
```c++
template<typename Sy=S>
S dot (const vvec<Sy>& v) const
```

The **cross product** is defined here only for `vvec` of size 3.

If `N` is 2, then v x w is defined to be v_x w_y - v_y w_x and for N=3, see your nearest vector maths textbook. The function signatures are
```c++
template<typename Sy=S>
S cross (const vvec<Sy>& w) const;
```
